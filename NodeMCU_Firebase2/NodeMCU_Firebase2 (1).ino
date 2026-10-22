#include <common.h>
#include <FB_Error.h>
#include <FB_Net.h>
#include <Firebase.h>
#include <FirebaseFS.h>
#include <Firebase_ESP_Client.h>
#include <MB_File.h>
#include <Utils.h>




//librerias
#include <ESP8266WiFi.h>

//------------------------------

// fiebase
#define FIREBASE_HOST "casa01-fc56a-default-rtdb.firebaseio.com" // databaseUrl SIN https://
#define FIREBASE_AUTH "ArH6EQVGtVofNdEHmCWoCwaxdcbGrY0NRkhUYKCL" // el secreto de la base
//------------------------------

// wifi credenciales
#define WIFI_SSID "Totalplay-77A5"    // nombre de la red wifi
#define WIFI_PASSWORD "274326638"   // contraseña de la red wifi
//  ------------------------------

// variables
const int LED1 = D5;
String foco1; // lee estado de firebase
const int LED2 = D6;
String foco2; // lee estado de firebase
const int LED3 = D7;
String foco3; // lee estado de firebase

void setup() {
  Serial.begin(9600); // velocidad
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("conectado !! ");

  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // CONFIGURAMOS----------------
  pinMode(LED1, OUTPUT);  // FOCO 1
  pinMode(LED2, OUTPUT);  // FOCO 2
  pinMode(LED3, OUTPUT);  // FOCO 3

}

void loop() {
  if (Firebase.failed()) {
    Serial.print("Error en conexion de firebase:");
    Serial.println(Firebase.error());
    delay(1000);
  }


  // FOCO1 ------------------------------------

  foco1 =  Firebase.getString("/casa1/foco1");
  Serial.println(foco1);
  if (foco1 == "ON" || foco1 == "on") {
    digitalWrite(LED1, HIGH);
    Serial.println("Foco 1 encendido");
  } else if (foco1 == "OFF" || foco1 == "off") {
    digitalWrite(LED1, LOW);
    Serial.println("Foco 1 apagado");
  }


  // FOCO2 ------------------------------------

  foco2 =  Firebase.getString("/casa1/foco2");
  Serial.println(foco2);
  if (foco2 == "ON" || foco2 == "on") {
    digitalWrite(LED2, HIGH);
    Serial.println("Foco 2 encendido");
  } else if (foco2 == "OFF" || foco2 == "off") {
    digitalWrite(LED2, LOW);
    Serial.println("Foco 2 apagado");
  }


  // FOCO3 ------------------------------------

  foco3 =  Firebase.getString("/casa1/foco3");
  Serial.println(foco3);
  if (foco3 == "ON" || foco3 == "on") {
    digitalWrite(LED3, HIGH);
    Serial.println("Foco 3 encendido");
  } else if (foco3 == "OFF" || foco3 == "off") {
    digitalWrite(LED3, LOW);
    Serial.println("Foco 3 apagado");
  }

  // fin FOCO1 ---------------------------------
}
