//===================================================================================
// PROYECTO NODEMCU ESP8266
// SENSOR DE MOVIMIENTO
// REGISTRO DE DATOS POR FIREBASE
//===================================================================================

//===================================================================================
// PANDA TECHNOLOGY
// MCCT. PEDRO HERNANDEZ OREGEL
// FEBRERO - 2021

// Enero 2022 - Actualización de librerías Firebase y Conexión WiFi
//===================================================================================

//------------------------------------------------------------
//   Librerias
//------------------------------------------------------------
//............................   ESP 8266   ..................................
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

//------------------------------------------------------------
//   Firebase
//------------------------------------------------------------
#define FIREBASE_HOST "casa01-fc56a-default-rtdb.firebaseio.com" // databaseUrl SIN https://
#define FIREBASE_AUTH "ArH6EQVGtVofNdEHmCWoCwaxdcbGrY0NRkhUYKCL" // el secreto de la base

// Se define el objeto FIREBASE y la BD de conexión
String path = "/casa1";
//Define un objeto de Firebase
FirebaseData firebaseData;

//------------------------------------------------------------
//   WiFi Credenciales
//------------------------------------------------------------
#define WIFI_SSID "Totalplay-77A5"    // nombre de la red wifi
#define WIFI_PASSWORD "274326638"   // contraseña de la red wifi

/*************************************************

   Public Constants

 *************************************************/

const byte LED1 = D2;
const byte LED2 = D3;
const byte PIR = D1;
bool bSensor;


//--------------------------------------------------------------------------------- FUNCTIONS
void printResult(FirebaseData &data);
void CausaError(void);
void InforSetLuzSensor(void);
//void InforGetLuzSensor(void);
void AvisoInicioPrograma(void);

//--------------------------------------------------------------------------------- SETUP

void setup() {

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(PIR, INPUT);
  delay(20000);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED2, HIGH);
    delay(500);
    digitalWrite(LED2, LOW);
  }
  Serial.println();
  Serial.print("Conectado con la IP: ");

  Serial.println(WiFi.localIP());
  
  // -------------------------------> Inicializar Conexión FIREBASE
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Establezca el tiempo de espera de lectura de la base de datos en 1 minuto (máximo 15 minutos)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);

  //Tamaño y  tiempo de espera de escritura, tiny (1s), small (10s), medium (30s) and large (60s).
  //tiny, small, medium, large and unlimited.
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  //--------------------------------------------------
  // Aviso de inicio de programa

  Serial.println("-----------------------------");
  Serial.println("SENSOR DE MOVIMIENTO - INICIALIZANDO");
  Serial.println("-----------------------------");
  AvisoInicioPrograma();
}

void loop() {
  
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Error en conexion de red");
    digitalWrite(LED2, HIGH);
    digitalWrite(LED1, LOW);
    delay(500);
    digitalWrite(LED2, LOW);
    delay(250);
  }
  else
  {
    //------------------------------------------------------
    // Cada Segundo se verifica si hay movimiento
    //------------------------------------------------------
    bSensor = digitalRead(PIR);
    Serial.print("Valor del Sensor: ");
    Serial.println(bSensor);
    delay(1000);
    if (bSensor)
    {
      Serial.println("Se detectó movimiento");
      if (Firebase.setInt(firebaseData, path + "/movimiento", 1)) 
      {
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, HIGH);
        delay(30000); // Espera 30 segundos
      } 
      else 
      {
        CausaError();
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, LOW);
        delay(30000); // Espera 30 segundos
      }   
    }
    else
    {
      Serial.println("Se envía mensaje");
      digitalWrite(LED2, LOW);
      if (Firebase.setInt(firebaseData, path + "/movimiento", 0)) {/*InforSetLuzSensor()*/;} else {CausaError();}      
    }    
  }
}

void AvisoInicioPrograma()
{
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  digitalWrite(LED1, HIGH);
  delay(500);
  digitalWrite(LED2, HIGH);
  delay(500);

  for (byte i = 0; i <= 2; i++)
  {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    delay(250);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    delay(250);
  }
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
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
// Proyecto Temperatura RIG (ProyectoNodeMCUTemperaturaRIG)
