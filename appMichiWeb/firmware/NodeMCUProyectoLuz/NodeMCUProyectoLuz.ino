// NodeMCU LuzCasa 2.3.1 — control local independiente de WiFi/Firebase.
// Derivado de NodeMCUProyectoLuz, Pedro Hernandez Oregel, 2021/2022.
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Schedule.h>
#include <time.h>
#include "DailySchedule.h"
#include "LocalControl.h"
#include "DHTD0.h"
#include "SensorReading.h"
#include "WifiPortal.h"
#include "secrets.h"

constexpr uint8_t RELAY[2]={D2,D1}, LED[2]={D6,D8}, BUTTON[2]={D5,D7};
DHTD0 sensor(D0,DHT11);
Michi::SensorReading reading;
uint32_t lastSensor=0,lastSensorPublish=0;
const char *SENSOR_PATH="/Farm/RigRojo";
const char *SCHEDULE_PATH="/casa1/horarioLuces";
const char *SCHEDULE_STATUS="/casa1/horarioLucesEstado";
FirebaseData firebaseData;
FirebaseData &scheduleData=firebaseData; // Un cliente TLS; solicitudes secuenciales.
LuzCasaWifi wifiSetup;
FirebaseConfig config;
FirebaseAuth auth;
Michi::DailySchedule dailySchedule;
Michi::ScheduleEngine dailyEngine;
Michi::LocalControl lights;
Michi::Button buttons[2];
bool scheduleConfigReady=false,wasConnected=false,firstSchedule=true,firstHeartbeat=true;
uint32_t lastSchedule=0,lastHeartbeat=0,lastLights=0,lastPublish=0,networkGate=0;
int64_t scheduleLastEvent=0;

void ledTick(){
 const bool online=wifiSetup.online();const uint32_t now=millis();
 for(uint8_t ch=0;ch<2;++ch)digitalWrite(LED[ch],Michi::offlineLedLevel(online,lights.on[ch],now)?HIGH:LOW);
}
void outputs(){
 for(uint8_t ch=0;ch<2;++ch)digitalWrite(RELAY[ch],lights.on[ch]?HIGH:LOW);
 ledTick();
}
int64_t currentEvent(){
 int64_t now=static_cast<int64_t>(time(nullptr));
 return now>=1700000000LL&&dailySchedule.valid()&&dailySchedule.enabled?Michi::latestEvent(dailySchedule,now).eventAt:0;
}
void localTick(){
 const uint32_t now=millis();
 for(uint8_t ch=0;ch<2;++ch)if(buttons[ch].sample(digitalRead(BUTTON[ch])==LOW,now)){
  lights.toggle(ch,currentEvent());outputs();
 }
 ledTick();
 // Sin red, espera ni registros dentro de esta tarea: corre tambien en yield().
}
void automaticTick(){
 Michi::Decision decision;
 if(dailyEngine.due(dailySchedule,static_cast<int64_t>(time(nullptr)),decision)){
  lights.automatic(decision.turnOn,decision.eventAt);outputs();
  dailyEngine.applied(decision);scheduleLastEvent=decision.eventAt;
 }
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
  if (!Firebase.getJSON(scheduleData, SCHEDULE_PATH)) { return; }
  wifiSetup.noteInternet(); // Conserva la ultima configuracion en una interrupcion breve.
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

void heartbeat(){
 const int64_t now=static_cast<int64_t>(time(nullptr));
 FirebaseJson status;
 status.set("schemaVersion",1);status.set("firmwareVersion","2.3.1");
 status.set("ready",now>=1700000000LL&&scheduleConfigReady);
 status.set("clockSynced",now>=1700000000LL);
 status.set("lastSeen",static_cast<double>(now)*1000.0);
 status.set("lastEventAt",static_cast<double>(scheduleLastEvent)*1000.0);
 status.set("enabled",dailySchedule.enabled);
 status.set("localControl",true);status.set("sensorPin","D0");status.set("sensorValid",reading.valid);
 if(Firebase.updateNode(firebaseData,SCHEDULE_STATUS,status))wifiSetup.noteInternet();
}
void publishLocal(){
 const uint8_t mask=lights.dirty;
 const uint32_t revision[2]={lights.revision[0],lights.revision[1]};
 FirebaseJson state;
 if(mask&1)state.set("foco1",lights.on[0]);
 if(mask&2)state.set("foco2",lights.on[1]);
 if(Firebase.updateNode(firebaseData,"/casa1",state)){wifiSetup.noteInternet();lights.acknowledge(mask,revision);}
}
void readRemote(){
 const uint32_t revision[2]={lights.revision[0],lights.revision[1]};
 if(!Firebase.getJSON(firebaseData,"/casa1"))return;
 wifiSetup.noteInternet();
 FirebaseJson &json=firebaseData.jsonObject();
 bool value;
 if(scheduleBool(json,"foco1",value))lights.remote(0,value,revision[0]);
 if(scheduleBool(json,"foco2",value))lights.remote(1,value,revision[1]);
 outputs();
}
void sensorTick(){
 const uint32_t now=millis();
 if(uint32_t(now-lastSensor)<10000)return;
 lastSensor=now;
 const float humidity=sensor.readHumidity(),temperature=sensor.readTemperature();
 reading.update(temperature,humidity);
 if(reading.valid)Serial.printf("LECTURA_LUZCASA: %.1f C / %.1f %%\n",reading.temperature,reading.humidity);
 else Serial.println(F("LECTURA_LUZCASA: ERROR_DHT11"));
}
void publishSensor(){
 FirebaseJson data;
 data.set("estado",reading.valid?"lectura_valida":"error_sensor");
 if(reading.valid){
  data.set("Temperatura",reading.temperature);data.set("Humedad",reading.humidity);
  data.set("actualizadoEn/.sv","timestamp");
 }else{
  // No mantener numeros antiguos como si fueran mediciones actuales.
  data.set("Temperatura");data.set("Humedad");
 }
 if(Firebase.updateNode(firebaseData,SENSOR_PATH,data)){wifiSetup.noteInternet();reading.pending=false;}
}
void networkTick(){
 const uint32_t now=millis();
 if(WiFi.status()!=WL_CONNECTED){wasConnected=false;return;}
 if(!wasConnected){
  wasConnected=true;firstSchedule=true;firstHeartbeat=true;
  Serial.println(F("WiFi conectado; sincronizando cambios locales."));
 }
 if(uint32_t(now-networkGate)<250)return;
 networkGate=now;
 // Primero publicar cambios pendientes. No se leen valores remotos antiguos.
 if(lights.dirty){
  if(uint32_t(now-lastPublish)>=1000){lastPublish=now;publishLocal();}
  return;
 }
 if(firstSchedule||uint32_t(now-lastSchedule)>=15000){
  firstSchedule=false;lastSchedule=now;readDailySchedule();return;
 }
 if(firstHeartbeat||uint32_t(now-lastHeartbeat)>=30000){
  firstHeartbeat=false;lastHeartbeat=now;heartbeat();return;
 }
 if(reading.pending&&uint32_t(now-lastSensorPublish)>=5000){
  lastSensorPublish=now;publishSensor();return;
 }
 if(uint32_t(now-lastLights)>=2000){lastLights=now;readRemote();}
}
void setup(){
 Serial.begin(115200);sensor.begin();
 for(uint8_t ch=0;ch<2;++ch){
  digitalWrite(RELAY[ch],LOW);pinMode(RELAY[ch],OUTPUT);
  digitalWrite(LED[ch],HIGH);pinMode(LED[ch],OUTPUT);
  pinMode(BUTTON[ch],INPUT_PULLUP);
 }
 const bool registered=schedule_recurrent_function_us([](){localTick();return true;},5000);
 Serial.println(registered?F("LuzCasa 2.3.1: botones activos sin Internet."):F("ERROR tarea local; se mantiene lectura desde loop."));
 config.database_url=FIREBASE_HOST;config.signer.tokens.legacy_token=FIREBASE_AUTH;
 config.timeout.socketConnection=3000;config.timeout.serverResponse=3000;config.timeout.sslHandshake=5000;
 Firebase.begin(&config,&auth);Firebase.reconnectWiFi(true);
 firebaseData.setBSSLBufferSize(4096,1024);
 Firebase.setReadTimeout(firebaseData,3000);Firebase.setwriteSizeLimit(firebaseData,"tiny");
 wifiSetup.begin(WIFI_SSID,WIFI_PASSWORD);
 configTime(0,0,"pool.ntp.org","time.google.com");
 // No se espera a WiFi, NTP o Firebase para atender los botones.
}
void loop(){localTick();wifiSetup.tick();automaticTick();sensorTick();networkTick();localTick();delay(1);}
