// Orizaba130: DHT11 D0; rele 1 D2 = foco3; rele 2 D1 = foco4.
// Comparte Firebase con el proyecto 1; usa un horario independiente.
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <time.h>
#include "DHTD0.h"
#include "DailySchedule.h"
#include "secrets.h"
const uint8_t pinReleA = D2, pinReleB = D1;
DHTD0 sensor(D0, DHT11);
FirebaseData firebaseData, scheduleData;
FirebaseConfig michiFirebaseConfig;
FirebaseAuth michiFirebaseAuth;
Michi::DailySchedule dailySchedule;
Michi::ScheduleEngine dailyEngine;
bool scheduleConfigReady=false, scheduleWritePending=false, scheduleFirstPoll=true;
unsigned long scheduleLastPoll=0, scheduleLastHeartbeat=0, lastSensor=0, lastManual=0, lastWiFiRetry=0, lastReport=0;
int64_t scheduleLastEvent=0;
const char *SCHEDULE_PATH="/casa1/horarioLuces34";
const char *SCHEDULE_STATUS="/casa1/horarioLuces34Estado";
WiFiEventHandler disconnectHandler;
const char *wifiStatusLabel() {
  switch (WiFi.status()) {
    case WL_CONNECTED: return "OK";
    case WL_NO_SSID_AVAIL: return "RED_NO_ENCONTRADA";
    case WL_CONNECT_FAILED: return "CONEXION_FALLIDA";
    case WL_CONNECTION_LOST: return "CONEXION_PERDIDA";
    case WL_DISCONNECTED: return "DESCONECTADO";
    default: return "CONECTANDO";
  }
}
const char *disconnectLabel(WiFiDisconnectReason reason) {
  switch (reason) {
    case WIFI_DISCONNECT_REASON_NO_AP_FOUND: return "RED_NO_ENCONTRADA";
    case WIFI_DISCONNECT_REASON_AUTH_FAIL: return "AUTENTICACION_RECHAZADA";
    case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT: return "NEGOCIACION_AGOTADA";
    default: return "DESCONEXION";
  }
}
void serviceDailySchedule();
void setup() {
  digitalWrite(pinReleA, LOW); digitalWrite(pinReleB, LOW);
  pinMode(pinReleA, OUTPUT); pinMode(pinReleB, OUTPUT);
  Serial.begin(115200); sensor.begin();
  disconnectHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event) {
    Serial.printf("WIFI_DIAGNOSTICO: %s codigo=%u\n", disconnectLabel(event.reason), static_cast<unsigned>(event.reason));
  });
  WiFi.persistent(false); WiFi.mode(WIFI_STA); WiFi.setAutoReconnect(true); WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  michiFirebaseConfig.database_url=FIREBASE_HOST;
  michiFirebaseConfig.signer.tokens.legacy_token=FIREBASE_AUTH;
  Firebase.begin(&michiFirebaseConfig,&michiFirebaseAuth); Firebase.reconnectWiFi(true);
  firebaseData.setBSSLBufferSize(4096,1024); scheduleData.setBSSLBufferSize(4096,1024);
  Firebase.setReadTimeout(firebaseData,5000); Firebase.setReadTimeout(scheduleData,5000);
  configTime(0,0,"pool.ntp.org","time.google.com");
  Serial.println("Orizaba130: sensor D0, focos 3/4, horario independiente.");
}
void loop() {
  serviceDailySchedule();
  const unsigned long ms=millis();
  const bool connected=WiFi.status()==WL_CONNECTED && Firebase.ready();
  if (WiFi.status()!=WL_CONNECTED && ms-lastWiFiRetry>=30000UL) {
    lastWiFiRetry=ms; WiFi.reconnect();
  }
  if (ms-lastReport>=10000UL) {
    lastReport=ms;
    Serial.printf("ESTADO_ORIZABA: WiFi=%s Firebase=%s Reloj=%s\n",
      wifiStatusLabel(),
      connected ? "OK" : "ESPERANDO", time(nullptr)>=1700000000LL ? "OK" : "ESPERANDO");
  }
  if (connected && !scheduleWritePending && ms-lastManual>=2000UL) {
    lastManual=ms;
    if (Firebase.getBool(firebaseData,"/casa1/foco3") && firebaseData.dataType()=="boolean")
      digitalWrite(pinReleA,firebaseData.boolData()?HIGH:LOW);
    if (Firebase.getBool(firebaseData,"/casa1/foco4") && firebaseData.dataType()=="boolean")
      digitalWrite(pinReleB,firebaseData.boolData()?HIGH:LOW);
  }
  // El sensor se comprueba tambien cuando no hay WiFi; solo se publica una lectura valida.
  if (ms-lastSensor>=10000UL) {
    lastSensor=ms;
    float humidity=sensor.readHumidity(), temperature=sensor.readTemperature();
    if (!isnan(humidity) && !isnan(temperature) && humidity>=0 && humidity<=100) {
      Serial.printf("LECTURA_LOCAL: %.1f C / %.1f %%\n",temperature,humidity);
      if (connected) {
        FirebaseJson readings;
        readings.set("Temperatura",temperature); readings.set("Humedad",humidity);
        readings.set("estado","lectura_valida");
        readings.set("actualizadoEn/.sv","timestamp");
        Serial.println(Firebase.updateNode(firebaseData,"/Farm/Orizaba130",readings)
          ? "FIREBASE_LECTURA: OK" : "FIREBASE_LECTURA: ERROR");
      }
    } else {
      Serial.println("LECTURA_LOCAL: ERROR_DHT11");
      if (connected) Firebase.setString(firebaseData,"/Farm/Orizaba130/estado","error_sensor");
    }
  }
  delay(10);
}

bool scheduleInt(FirebaseJson &json, const char *key, int &value) {
  FirebaseJsonData data; json.get(data, key);
  if (!data.success || data.typeNum != FirebaseJson::JSON_INT) return false;
  value = data.to<int>(); return true;
}
bool scheduleBool(FirebaseJson &json, const char *key, bool &value) {
  FirebaseJsonData data; json.get(data, key);
  if (!data.success || data.typeNum != FirebaseJson::JSON_BOOL) return false;
  value = data.to<bool>(); return true;
}
void readDailySchedule() {
  if (!Firebase.getJSON(scheduleData, SCHEDULE_PATH)) { scheduleConfigReady = false; return; } // Conserva la ultima configuracion en una interrupcion breve.
  FirebaseJson &json = scheduleData.jsonObject();
  FirebaseJsonData zone; json.get(zone, "timeZone");
  Michi::DailySchedule next;
  int schema = 0;
  bool correct = zone.success && zone.to<String>() == "America/Mexico_City" &&
    scheduleInt(json, "schemaVersion", schema) && schema == 1 &&
    scheduleBool(json, "configured", next.configured) && scheduleBool(json, "enabled", next.enabled) &&
    scheduleInt(json, "on/hour", next.onHour) && scheduleInt(json, "on/minute", next.onMinute) &&
    scheduleInt(json, "off/hour", next.offHour) && scheduleInt(json, "off/minute", next.offMinute);
  const bool bounds = next.onHour >= 0 && next.onHour < 24 && next.offHour >= 0 && next.offHour < 24 &&
    next.onMinute >= 0 && next.onMinute < 60 && next.offMinute >= 0 && next.offMinute < 60;
  if (!correct || !bounds || (next.enabled && !next.valid())) {
    dailySchedule.enabled = false; scheduleConfigReady = false; dailyEngine.reset(); return;
  }
  if (!(next == dailySchedule)) dailyEngine.reset();
  dailySchedule = next; scheduleConfigReady = true;
}
void publishScheduleHeartbeat(int64_t now) {
  FirebaseJson status;
  const bool synced = now >= 1700000000LL;
  status.set("schemaVersion", 1);
  status.set("firmwareVersion", "orizaba130-1.0.1");
  status.set("ready", synced && scheduleConfigReady);
  status.set("clockSynced", synced);
  status.set("lastSeen", static_cast<double>(now) * 1000.0);
  status.set("lastEventAt", static_cast<double>(scheduleLastEvent) * 1000.0);
  status.set("enabled", dailySchedule.enabled);
  if (Firebase.updateNode(scheduleData, SCHEDULE_STATUS, status))
    Serial.println(synced && scheduleConfigReady ? "HORARIO_DISPOSITIVO: LISTO" : "HORARIO_DISPOSITIVO: ESPERANDO");
  else Serial.println("HORARIO_DISPOSITIVO: ERROR_FIREBASE");
}
void serviceDailySchedule() {
  const bool wifi = WiFi.status() == WL_CONNECTED && Firebase.ready();
  const unsigned long ms = millis();
  if (wifi && (scheduleFirstPoll || ms - scheduleLastPoll >= 15000UL)) {
    scheduleFirstPoll = false; scheduleLastPoll = ms; readDailySchedule();
  }
  const int64_t utcNow = static_cast<int64_t>(time(nullptr));
  Michi::Decision decision;
  if (dailyEngine.due(dailySchedule, utcNow, decision)) {
    // Mismos pines y polaridad que el programa original.
    digitalWrite(pinReleA, decision.turnOn ? HIGH : LOW);
    digitalWrite(pinReleB, decision.turnOn ? HIGH : LOW);
    dailyEngine.applied(decision); scheduleLastEvent = decision.eventAt;
    scheduleWritePending = true;
  }
  if (wifi && scheduleWritePending) {
    FirebaseJson states;
    states.set("foco3", digitalRead(pinReleA) == HIGH);
    states.set("foco4", digitalRead(pinReleB) == HIGH);
    if (Firebase.updateNode(scheduleData, "/casa1", states)) scheduleWritePending = false;
  }
  if (wifi && (scheduleLastHeartbeat == 0 || ms - scheduleLastHeartbeat >= 30000UL)) {
    scheduleLastHeartbeat = ms; publishScheduleHeartbeat(utcNow);
  }
}
