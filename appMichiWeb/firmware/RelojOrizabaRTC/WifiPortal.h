#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include "WifiProfile.h"
#include "secrets.h"

class RelojWifi {
 ESP8266WebServer web{80};
 Michi::WifiProfile saved{},candidate{};
 const char *fallbackName=nullptr,*fallbackKey=nullptr;
 bool ap=false,testing=false,queued=false,haveInternet=false;
 uint32_t started=0,probeAt=0,lastGood=0,queuedAt=0,apAttemptAt=0;
 String token;
 const char *result="";
 bool local(){return ap&&web.client().localIP()==WiFi.softAPIP();}
 void connectPrevious(){
  if(saved.valid())WiFi.begin(saved.ssid,saved.password);
  else WiFi.begin(fallbackName,fallbackKey);
 }
 bool probe(const char *url){
  WiFiClient client;HTTPClient http;
  client.setTimeout(1000);http.setTimeout(1000);
  if(!http.begin(client,url))return false;
  http.setReuse(false);int code=http.GET();http.end();return code==204;
 }
 void open(){
  if(ap)return;
  WiFi.mode(WIFI_AP_STA);
  apAttemptAt=millis();
  if(!WiFi.softAP("OrizabaReloj-Config",CONFIG_AP_PASSWORD,1,false,4)){
   Serial.println(F("CONFIG WIFI: no se pudo iniciar AP; se reintentara."));return;
  }
  ap=true;token=String(ESP.random(),HEX)+String(ESP.random(),HEX);
  Serial.println(F("CONFIG WIFI: OrizabaReloj-Config - abrir http://192.168.4.1"));
 }
public:
 bool online()const{return WiFi.status()==WL_CONNECTED&&haveInternet&&uint32_t(millis()-lastGood)<45000;}
 void noteInternet(){haveInternet=true;lastGood=millis();}
 void begin(const char *name,const char *key){
  fallbackName=name;fallbackKey=key;
  EEPROM.begin(sizeof(Michi::WifiProfile)+4);EEPROM.get(0,saved);
  WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);
  open();connectPrevious();started=millis();probeAt=started-20000UL;
  web.on("/",HTTP_GET,[this](){
   if(!local()){web.send(403,"text/plain","Conectate a OrizabaReloj-Config.");return;}
   String page=F("<!doctype html><html lang='es'><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>WiFi del reloj Orizaba130</title><style>body{font:18px sans-serif;max-width:420px;margin:25px auto;padding:20px;background:#edf7ed}label{display:block}input,button{box-sizing:border-box;width:100%;padding:12px;margin:8px 0}</style><h1>WiFi del reloj Orizaba130</h1><p>Usa una red de 2.4 GHz. El reloj conserva la hora con el DS3231.</p><form method='post' action='/guardar'><input type='hidden' name='token' value='");
   page+=token;page+=F("'><label>Nombre de red<input name='ssid' maxlength='32' required></label><label>Contrasena<input type='password' name='password' maxlength='64' autocomplete='new-password'></label><button>Guardar y conectar</button></form><p id='estado'>");
   page+=result;page+=F("</p><script>setInterval(()=>fetch('/estado').then(r=>r.text()).then(t=>document.getElementById('estado').textContent=t).catch(()=>{document.getElementById('estado').textContent='Si se desconecto la red de configuracion, vuelve a tu WiFi habitual y revisa los LED.'}),3000)</script></html>");
   web.sendHeader("Cache-Control","no-store");web.send(200,"text/html; charset=utf-8",page);
  });
  web.on("/estado",HTTP_GET,[this](){if(!local()){web.send(403);return;}web.sendHeader("Cache-Control","no-store");web.send(200,"text/plain; charset=utf-8",result);});
  web.on("/guardar",HTTP_POST,[this](){
   if(!local()||web.arg("token")!=token){web.send(403,"text/plain","Solicitud no valida.");return;}
   if(testing||queued){web.send(409,"text/plain","Espera a que termine la conexion.");return;}
   if(!candidate.assign(web.arg("ssid").c_str(),web.arg("password").c_str())){web.send(400,"text/plain","Revisa la red y contrasena (8 a 64 caracteres o vacia para red abierta).");return;}
   queued=true;queuedAt=millis();
   result="Probando red. Espera hasta 30 segundos. Los datos anteriores se conservan hasta conectar.";
   web.sendHeader("Location","/",true);web.send(303,"text/plain","");
  });
  web.begin();
 }
 void tick(){
  uint32_t now=millis();
  if(ap&&!(WiFi.getMode()&WIFI_AP))ap=false;
  if(!ap&&uint32_t(now-apAttemptAt)>=5000)open();
  if(ap)web.handleClient();
  if(queued&&uint32_t(now-queuedAt)>=500){
   queued=false;testing=true;started=now;haveInternet=false;
   WiFi.begin(candidate.ssid,candidate.password);
   probeAt=now-20000UL;
  }
  if(testing){
   if(WiFi.status()==WL_CONNECTED&&WiFi.SSID()==String(candidate.ssid)){
    EEPROM.put(0,candidate);
    if(EEPROM.commit()){saved=candidate;result="Red guardada. Comprobando acceso a Internet...";}
    else result="Conecto, pero no se pudo guardar. Vuelve a intentarlo.";
    testing=false;
   }else if(uint32_t(now-started)>=30000){
    testing=false;result="No conecto: revisa nombre, clave y banda 2.4 GHz. Se conserva la red anterior.";
    connectPrevious();
   }
  }
  if(WiFi.status()!=WL_CONNECTED){haveInternet=false;}
  else if(!testing&&!queued&&uint32_t(now-probeAt)>=20000){
   probeAt=now;
   // Solo respuestas 204; no se envian datos privados ni se siguen redirecciones.
   if(probe("http://connectivitycheck.gstatic.com/generate_204")||probe("http://cp.cloudflare.com/generate_204"))noteInternet();
  }
  if(online()&&!testing&&!queued){
   result="Internet disponible. La red OrizabaReloj-Config permanece disponible para cambiar el WiFi.";
  }

 }
};
