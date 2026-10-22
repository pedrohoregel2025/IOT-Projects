//===================================================================================
// PROYECTO NODEMCU ESP8266
// SENSOR DE TEMPERATURA
// REGISTRO VISUAL POR DISPLAY LCD 16x2
// REGISTRO DE DATOS POR FIREBASE
//===================================================================================

//===================================================================================
// MCCT. PEDRO HERNANDEZ OREGEL
// ENERO - 2022
//===================================================================================


//--------------------------------------------------------------------------------- INCLUDE
//............................   LiquidCrystal I2C   ..................................
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//............................   ESP 8266   ..................................
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

//............................   DHT11   ..................................
#include "DHTesp.h"

//---------------------------------------------------------------------------------- DEFINE & GLOBAL

// Se crea el objeto DHT11
DHTesp dht;
#define PINDHT D3

//Crear el objeto lcd  dirección 0x27, 16 columnas x 2 filas
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Se crean los parametros para la conexión a FIREBASE
// Ruta de la base de datos y secreto de la BD
#define FIREBASE_HOST "casa01-fc56a-default-rtdb.firebaseio.com" //Sin http:// o https:// 
#define FIREBASE_AUTH "ArH6EQVGtVofNdEHmCWoCwaxdcbGrY0NRkhUYKCL"

// Se define el objeto FIREBASE y la BD de conexión
String path = "/Farm";
//Define un objeto de Firebase
FirebaseData firebaseData;

// Se definen los parámetros para la coenxión WiFi
#define WIFI_SSID "Totalplay-77A5"
#define WIFI_PASSWORD "274326638"

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;     // will store last time LED was updated

// constants won't change:
const long interval = 2000;           // interval (milliseconds)

//--------------------------------------------------------------------------------- FUNCTIONS
void printResult(FirebaseData &data);
void CausaError(void);
void InforSetLuzSensor(void);
void InforGetLuzSensor(void);

//--------------------------------------------------------------------------------- SETUP
void setup()
{
  // -------------------------------> Inicializar el LCD
  lcd.init();
  //Encender la luz de fondo.
  lcd.backlight();

  // -------------------------------> Inicializar DHT11
  // Conecta el sensor DHT11 al GPIO 1 (pin D3)
  dht.setup(PINDHT, DHTesp::DHT11);

  // -------------------------------> Inicializar Conexión WiFi
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a ....");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Conectado con la IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // -------------------------------> Inicializar Conexión FIREBASE
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Establezca el tiempo de espera de lectura de la base de datos en 1 minuto (máximo 15 minutos)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);

  //Tamaño y  tiempo de espera de escritura, tiny (1s), small (10s), medium (30s) and large (60s).
  //tiny, small, medium, large and unlimited.
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}


//---------------------------------------------------------------------------------- LOOP
void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    delay(dht.getMinimumSamplingPeriod());
    float humedad = dht.getHumidity();
    float temperatura = dht.getTemperature();
    Serial.print(dht.getStatusString());
    Serial.print("\t");
    Serial.print(String(humedad));
    Serial.print("\t\t");
    Serial.println(String(temperatura));

    lcd.setCursor(0, 0);
    lcd.print("Temp = " + String(temperatura) + " C");
    lcd.setCursor(0, 1);
    lcd.print("Hume = " + String(humedad) + " %");

    Serial.println("------------------------------------");
    Serial.println("ACTUALIZAR EL ESTADO DE LOS SENSORES");
    if (Firebase.setFloat(firebaseData, path + "/RigAzul/Humedad", humedad)) {InforSetLuzSensor();} else {CausaError();}
    if (Firebase.setFloat(firebaseData, path + "/RigAzul/Temperatura", temperatura)) {InforSetLuzSensor();} else {CausaError();}
    if (Firebase.setFloat(firebaseData, path + "/RigRojo/Humedad", humedad)) {InforSetLuzSensor();} else {CausaError();}
    if (Firebase.setFloat(firebaseData, path + "/RigRojo/Temperatura", temperatura)) {InforSetLuzSensor();} else {CausaError();}
  }
}

//---------------------------------------------------------------------------------- InforGetLuzSensor

void InforGetLuzSensor(void)
{
  Serial.println("Aprobado");
  Serial.println("Ruta: " + firebaseData.dataPath());
  Serial.println("Tipo: " + firebaseData.dataType());
  Serial.println("ETag: " + firebaseData.ETag());
  Serial.print("Valor: ");
  printResult(firebaseData);
  Serial.println("------------------------------------");
  Serial.println(); 
}

//---------------------------------------------------------------------------------- InforSetLuzSensor
void InforSetLuzSensor(void)
{
  Serial.println("Aprobado");
  Serial.println("Ruta: " + firebaseData.dataPath());
  Serial.println("Tipo: " + firebaseData.dataType());
  Serial.println("ETag: " + firebaseData.ETag());
  Serial.print("Valor: ");
  printResult(firebaseData);
  Serial.println("------------------------------------");
  Serial.println(); 
}

//---------------------------------------------------------------------------------- CausaError
void CausaError(void)
{
  Serial.println("ERROR");
  Serial.println("RAZON: " + firebaseData.errorReason());
  Serial.println("------------------------------------");
  Serial.println();
}

//---------------------------------------------------------------------------------- printResult
void printResult(FirebaseData &data)
{
    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
}


//---------------------------------------------------------------------------------------------------------
// REFERENCIAS
//---------------------------------------------------------------------------------------------------------
//
// Instalación de librerías para Gestión de Placas
// https://naylampmechatronics.com/blog/56_usando-esp8266-con-el-ide-de-arduino.html
//
// Instalación y configuración de librerías de sensor de temperatura DHT11
// https://smelpro.com/blog/sensor-dht11/
//
// Interactuar Sensor DHT11 y ESP8266
// https://smelpro.com/blog/esp8266-webserver-dht11/
//
// Esta versión utiliza una librería para DHT11 con ESP8266, todo incluido
// https://www.ac.uma.es/~andres/tutorial/
//
// Conexión y configuración LCD configurado con I2C
// https://naylampmechatronics.com/blog/35_tutorial-lcd-con-i2c-controla-un-lcd-con-solo-dos-pines.html
// https://elosciloscopio.com/tutorial-pantalla-lcd-arduino-esp8266-esp32/
//
// Ejemplo de función e implementación de millis()
// https://docs.arduino.cc/built-in-examples/digital/BlinkWithoutDelay
//
// Descarga e instalación de librerias para Firebase
// https://www.youtube.com/watch?v=ERNcbSQA37E
// https://cursos.innovadomotics.com/cursos/firebase-y-arduino/leccion/firebase-iot/
// 1_ArdFireBasico.ino
