 /*
 * DIMMER AZUL 2 CANALES
 GND - GND
 VCC - 5V
 PD2 - D7
 PZC - D6
*/
#include "hw_timer.h" 
#include <ESP8266WiFi.h>
#include "FirebaseArduino.h"


// credenciales firebase
#define FIREBASE_HOST "" // url firebase sin https://
#define FIREBASE_AUTH "" //Secreto de la base de datos

// wifi credenciales
#define WIFI_SSID "" // nombre de la red
#define WIFI_PASSWORD "" //contraseña de la red
//------------------------------


int anterior = 999;
int dimmer;
int val;
int resetBot = 14; // este pin manda el pulso al pin RST (PARA HACER RESET)
 
const byte zcPin = 12; // D6
const byte pwmPin = 13; // D7


byte fade = 0;
byte state = 1;
byte tarBrightness = 255;
byte curBrightness = 0;
byte zcState = 0; // 0 = ready; 1 = processing;
void setup() {
  Serial.begin(115200);
 // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.print("connected:fer ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  delay(1000);

  //-----------

     
  pinMode(zcPin, INPUT_PULLUP);
  pinMode(pwmPin, OUTPUT);
  attachInterrupt(zcPin, zcDetectISR, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  hw_timer_init(NMI_SOURCE, 0);
  hw_timer_set_func(dimTimerISR);
}

void loop() {


     dimmer = Firebase.getFloat("/casa1/dimmer"); 
    if(dimmer != anterior){
        Serial.print("envia al arduino: ");
       // Serial.println(dimmer);
          val = dimmer;
          tarBrightness =val;
         
        Serial.println(dimmer);
        anterior=dimmer;
      }else{
       // no manda nada
        }


    if (Firebase.failed()) {
        
        Serial.print("Error en connexion de firebase:");
        Serial.println(Firebase.error());
        delay(1000);
     // el super reset (si hay error en firebase hace un reset)
           pinMode(resetBot, OUTPUT);
           delay(2000);
           pinMode(resetBot, INPUT);
     //end super reset
       // return;    
  }

        

}


void dimTimerISR() {
    if (fade == 1) {
      if (curBrightness > tarBrightness || (state == 0 && curBrightness > 0)) {
        --curBrightness;
      }
      else if (curBrightness < tarBrightness && state == 1 && curBrightness < 255) {
        ++curBrightness;
      }
    }
    else {
      if (state == 1) {
        curBrightness = tarBrightness;
      }
      else {
        curBrightness = 0;
      }
    }
    
    if (curBrightness == 0) {
      state = 0;
      digitalWrite(pwmPin, 0);
    }
    else if (curBrightness == 255) {
      state = 1;
      digitalWrite(pwmPin, 1);
    }
    else {
      digitalWrite(pwmPin, 1);
    }
    
    zcState = 0;
}

void zcDetectISR() {
  if (zcState == 0) {
    zcState = 1;
  
    if (curBrightness < 255 && curBrightness > 0) {
      digitalWrite(pwmPin, 0);
      
      int dimDelay = 30 * (255 - curBrightness) + 400;//400
      hw_timer_arm(dimDelay);
    }
  }
}
