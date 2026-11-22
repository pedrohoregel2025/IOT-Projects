// Reloj Orizaba130 RTC 1.0.2. Solo sensor/reloj: no controla focos ni horarios de reles.
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <MD_MAX72xx.h>
#include <Schedule.h>
#include "DHTD0.h"
#include "SensorReading.h"
#include "LocalControl.h"
#include "ManualSelection.h"
#include "ClockService.h"
#include "MatrixAnimations.h"
#include "WifiPortal.h"
#include "secrets.h"
#include "Brightness.h"
MD_MAX72XX matrix(MD_MAX72XX::FC16_HW,D7,D5,D6,4);
DHTD0 dht(D0,DHT11);
ClockService clockService;
RelojWifi wifi;
Michi::SensorReading reading;
Michi::Button button;
Reloj::CalendarSchedule schedule;
FirebaseData firebase;
FirebaseConfig config;FirebaseAuth auth;
constexpr uint8_t BUTTON_PIN=D3;
uint8_t brightness=0;
Reloj::BrightnessMemory brightnessMemory;
uint32_t brightnessAt=0,brightnessAckAt=0;
bool brightnessAckPending=true;
uint8_t pixels[32]={};
Reloj::ManualSelection selection;
uint8_t scene=0,previousAnimation=255,animation=0;
bool feedbackShown=false;
uint8_t observedButton=HIGH;
uint16_t animationStep=0;
bool overlay=false,sensorSequence=false;
uint8_t presses=0;
uint32_t renderAt=0,sensorAt=0,publishedAt=0,wifiMessageAt=0,scrollAt=0;
int scrollX=32;char text[110]={};
int64_t utcNow=0;
void buttonTick(){observedButton=digitalRead(BUTTON_PIN);if(button.sample(observedButton==LOW,millis())&&presses<8)++presses;}
void flush(){matrix.control(MD_MAX72XX::UPDATE,MD_MAX72XX::OFF);for(uint8_t x=0;x<32;++x)matrix.setColumn(31-x,pixels[x]);matrix.control(MD_MAX72XX::UPDATE,MD_MAX72XX::ON);}
int width(const char *message,int gap=1){int total=0;uint8_t glyph[8];while(*message)total+=matrix.getChar(*message++,8,glyph)+gap;return total?total-gap:0;}
void draw(const char *message,int x,int gap=1){
 memset(pixels,0,32);uint8_t glyph[8];
 while(*message){uint8_t n=matrix.getChar(*message++,8,glyph);for(uint8_t i=0;i<n;++i,++x)if(x>=0&&x<32)pixels[x]=glyph[i];x+=gap;}flush();
}
void makeText(uint8_t mode){
 if(mode==1){if(reading.valid)snprintf(text,sizeof(text),"Temperatura %.1f C",reading.temperature);else snprintf(text,sizeof(text),"DHT11 sin lectura valida");}
 if(mode==2){if(reading.valid)snprintf(text,sizeof(text),"Humedad %.0f %%",reading.humidity);else snprintf(text,sizeof(text),"DHT11 sin lectura valida");}
 if(mode==3){
  if(!Reloj::validEpoch(utcNow)){snprintf(text,sizeof(text),"Calendario sin hora valida");return;}
  time_t local=static_cast<time_t>(utcNow-21600);tm value{};gmtime_r(&local,&value);
  static const char* days[]={"Domingo","Lunes","Martes","Miercoles","Jueves","Viernes","Sabado"};
  static const char* months[]={"enero","febrero","marzo","abril","mayo","junio","julio","agosto","septiembre","octubre","noviembre","diciembre"};
  snprintf(text,sizeof(text),"%s %d de %s de %d",days[value.tm_wday],value.tm_mday,months[value.tm_mon],value.tm_year+1900);
 }
 if(mode==5)snprintf(text,sizeof(text),"Configurar Red Wifi");
}
void startScene(uint8_t mode,bool automatic=false){
 scene=mode;overlay=automatic;scrollX=32;scrollAt=millis();renderAt=0;animationStep=0;
 makeText(mode);
}
void restoreBase(){sensorSequence=false;startScene(selection.mode);}
void sensorTick(){
 if(uint32_t(millis()-sensorAt)<10000)return;sensorAt=millis();
 const float h=dht.readHumidity(),t=dht.readTemperature();reading.update(t,h);
 if(reading.valid)Serial.printf("ORIZABA_RELOJ: %.1f C / %.1f %%\n",t,h);else Serial.println(F("ORIZABA_RELOJ: error DHT11"));
}
void publishTick(){
 if(!reading.pending||WiFi.status()!=WL_CONNECTED||uint32_t(millis()-publishedAt)<10000)return;
 publishedAt=millis();FirebaseJson data;
 data.set("estado",reading.valid?"lectura_valida":"error_sensor");
 data.set("origen","reloj-orizaba-rtc-1.0.2");
 if(reading.valid){data.set("Temperatura",reading.temperature);data.set("Humedad",reading.humidity);data.set("actualizadoEn/.sv","timestamp");}
 else {data.set("Temperatura");data.set("Humedad");}
 if(Firebase.updateNode(firebase,"/Farm/Orizaba130",data)){reading.pending=false;wifi.noteInternet();Serial.println(F("FIREBASE_SENSOR: OK"));}
 else Serial.println(F("FIREBASE_SENSOR: ERROR"));
}
// Configuracion independiente de las mediciones; EEPROM comparte el buffer del portal.
void brightnessTick(){
 if(WiFi.status()!=WL_CONNECTED)return;
 uint32_t now=millis();
 if(uint32_t(now-brightnessAt)>=5000){
  brightnessAt=now;
  if(Firebase.getInt(firebase,"/dispositivos/Orizaba130/matriz/brillo")&&firebase.dataType()=="int"){
   int desired=firebase.intData();
   if(Reloj::validBrightness(desired)){
    wifi.noteInternet();
    if(brightness!=desired){brightness=desired;matrix.control(MD_MAX72XX::INTENSITY,brightness);brightnessAckPending=true;Serial.printf("BRILLO_MATRIX: %u/15\n",brightness);}
    if(!brightnessMemory.valid()||brightnessMemory.value!=brightness){
     Reloj::BrightnessMemory next;next.set(brightness);
     EEPROM.put(sizeof(Michi::WifiProfile),next);
     if(EEPROM.commit())brightnessMemory=next;
    }
   }
  }
 }
 if((brightnessAckPending&&uint32_t(now-brightnessAckAt)>=5000)||uint32_t(now-brightnessAckAt)>=30000){
  brightnessAckAt=now;FirebaseJson state;
  state.set("brillo",brightness);state.set("firmware","reloj-orizaba-rtc-1.0.2");state.set("actualizadoEn/.sv","timestamp");
  if(Firebase.updateNode(firebase,"/dispositivos/Orizaba130/matriz/estado",state)){brightnessAckPending=false;wifi.noteInternet();}
 }
}
void displayTick(){
 const uint32_t now=millis();utcNow=clockService.utc(wifi.online());schedule.tick(utcNow);
 if(presses){
  --presses;selection.next(now);sensorSequence=false;startScene(selection.mode);
  const char* names[]={"HORA","TEMP","HUM","FECHA"};
  draw(names[selection.mode],(32-width(names[selection.mode]))/2);feedbackShown=true;
  Serial.printf("BOTON_OK: vista=%s GPIO0=D3\n",names[selection.mode]);return;
 }
 if(selection.feedback(now))return;
 if(feedbackShown){feedbackShown=false;startScene(selection.mode);}

 if(!overlay&&selection.allowsAutomatic(now)){
  uint8_t event=schedule.take();
  if(event==Reloj::CalendarSchedule::Calendar){startScene(3,true);return;}
  if(event==Reloj::CalendarSchedule::Sensors){sensorSequence=true;startScene(1,true);return;}
  if(event==Reloj::CalendarSchedule::Animation){
   animation=previousAnimation==255?ESP.random()%5:(previousAnimation+1+ESP.random()%4)%5;previousAnimation=animation;
   startScene(4,true);return;
  }
  if(!wifi.online()&&uint32_t(now-wifiMessageAt)>=60000){wifiMessageAt=now;startScene(5,true);return;}
 }
 if(scene==0){
  if(uint32_t(now-renderAt)<250)return;renderAt=now;char value[8];
  if(Reloj::validEpoch(utcNow)){time_t local=static_cast<time_t>(utcNow-21600);tm t{};gmtime_r(&local,&t);snprintf(value,sizeof(value),"%02d:%02d",t.tm_hour,t.tm_min);}else snprintf(value,sizeof(value),"--:--");
  draw(value,(32-width(value,2))/2,2);return;
 }
 if(scene==4){
  if(uint32_t(now-renderAt)<40)return;renderAt=now;MatrixAnimations::render(animation,animationStep++,pixels);flush();
  if(animationStep>=MatrixAnimations::length(animation))restoreBase();return;
 }
 if(uint32_t(now-scrollAt)<65)return;scrollAt=now;draw(text,scrollX--);
 if(scrollX < -width(text)){
  if(sensorSequence&&scene==1){startScene(2,true);return;}
  if(overlay){restoreBase();return;}
  startScene(selection.mode); // Seleccion manual permanece hasta otra pulsacion.
 }
}
void reportTick(){
 static int reported=-1;
 if(reported!=observedButton){reported=observedButton;Serial.printf("BOTON_D3: %s\n",reported==LOW?"PULSADO_LOW":"REPOSO_HIGH");}
 static uint32_t last=0;if(uint32_t(millis()-last)<10000)return;last=millis();
 const char* sources[]={"SIN_HORA","NTP","DS3231","INTERNO"};
 Serial.printf("ESTADO_RELOJ: fuente=%s RTC=%s Internet=%s\n",sources[clockService.source],clockService.rtcHealthy()?"OK":"SIN_HORA_VALIDA",wifi.online()?"OK":"SIN_CONEXION");
}
void setup(){
 Serial.begin(115200);pinMode(BUTTON_PIN,INPUT_PULLUP);
 matrix.begin();matrix.control(MD_MAX72XX::INTENSITY,brightness);matrix.clear();dht.begin();
 if(!schedule_recurrent_function_us([](){buttonTick();return true;},5000))Serial.println(F("BOTON: tarea auxiliar no disponible; se conserva lectura desde loop"));
 Serial.println(F("RelojOrizabaRTC 1.0.2: matriz D7/D5/D6, DHT D0, RTC SDA D2 SCL D1, boton D3"));
 wifi.begin(WIFI_SSID,WIFI_PASSWORD);
 EEPROM.get(sizeof(Michi::WifiProfile),brightnessMemory);
 if(brightnessMemory.valid())brightness=brightnessMemory.value;
 matrix.control(MD_MAX72XX::INTENSITY,brightness);
 clockService.begin();
 config.database_url=FIREBASE_HOST;config.signer.tokens.legacy_token=FIREBASE_AUTH;
 config.timeout.socketConnection=2000;config.timeout.serverResponse=2000;config.timeout.sslHandshake=3000;
 Firebase.begin(&config,&auth);Firebase.reconnectWiFi(true);firebase.setBSSLBufferSize(4096,1024);
 Firebase.setReadTimeout(firebase,2000);Firebase.setwriteSizeLimit(firebase,"tiny");
 utcNow=clockService.utc(wifi.online());startScene(5,true);wifiMessageAt=millis();
}
void loop(){buttonTick();clockService.tick();displayTick();wifi.tick();sensorTick();publishTick();brightnessTick();displayTick();reportTick();delay(1);}
