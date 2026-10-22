//===================================================================================
// PROYECTO NODEMCU ESP8266
// CONTROL DE LUZ EXTERIOR
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
#define WIFI_PASSWORD "27432cc38"   // contraseña de la red wifi

/*************************************************

   Public Constants

 *************************************************/
byte pinBtnA = D5;//Variable de tipo entero que contiene el numero del pin al cual conectamos la señal de un botón
byte pinLedA = D6;//Variable de tipo entero que contiene el numero del pin al cual conectamos la señal de un led
byte pinBtnB = D7;//Variable de tipo entero que contiene el numero del pin al cual conectamos la señal de un botón
byte pinLedB = D8;//Variable de tipo entero que contiene el numero del pin al cual conectamos la señal de un led

byte pinReleA = D2;
byte pinReleB = D1;

int R;

//--------------------------------------------------------------------------------- FUNCTIONS
void printResult(FirebaseData &data);
void CausaError(void);
void InforSetLuzSensor(void);
//void InforGetLuzSensor(void);
void AvisoInicioPrograma(void);

//--------------------------------------------------------------------------------- SETUP

void setup() {

  Serial.begin(115200);//Iniciamos la comunicación serial
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  pinMode(pinBtnA, INPUT);//Configuramos el pin del boton como entrada
  pinMode(pinLedA, OUTPUT);//Configuramos el pin del led como salida
  pinMode(pinBtnB, INPUT);//Configuramos el pin del boton como entrada
  pinMode(pinLedB, OUTPUT);//Configuramos el pin del led como salida
  pinMode(pinReleA, OUTPUT);
  pinMode(pinReleB, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(pinLedB, HIGH);
    delay(500);
    digitalWrite(pinLedB, LOW);
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
    digitalWrite(pinLedB, HIGH);
    digitalWrite(pinLedA, LOW);
    delay(500);
    digitalWrite(pinLedB, LOW);
    delay(250);
  }
  else
  {
  
    if (digitalRead(pinBtnA) == 0 ) { //Si la lectura al pin del botón regresa un cero entramos al if y ejecutamos el código, si no, nos saltamos el código del if
      digitalWrite(pinLedA, !digitalRead(pinLedA) );//Obtenemos la lectura del pin del Led para escribir en el mismo pin el valor opuesto 0->1 1->0
      Serial.println( "El estado del led A ha cambiado" );//Imprimimos en el monitor serie
      digitalWrite(pinReleA, !digitalRead(pinReleA) );
      if (Firebase.setInt(firebaseData, path + "/foco1", !digitalRead(pinLedA))) {InforSetLuzSensor();} else {CausaError();}
      delay(500);//Esperamos 0.5 segundos antes de salir del if para evitar que un solo botonazo cause varios cambios de estado en el pin del led
    }

    if (digitalRead(pinBtnB) == 0 ) { //Si la lectura al pin del botón regresa un cero entramos al if y ejecutamos el código, si no, nos saltamos el código del if
      digitalWrite(pinLedB, !digitalRead(pinLedB) );//Obtenemos la lectura del pin del Led para escribir en el mismo pin el valor opuesto 0->1 1->0
      digitalWrite(pinReleB, !digitalRead(pinReleB) );
      Serial.println( "El estado del led B ha cambiado" );//Imprimimos en el monitor serie
      if (Firebase.setInt(firebaseData, path + "/foco2", !digitalRead(pinLedB))) {InforSetLuzSensor();} else {CausaError();}
      delay(500);//Esperamos 0.5 segundos antes de salir del if para evitar que un solo botonazo cause varios cambios de estado en el pin del led
    }
    
    Serial.println("------------------------------------");
    Serial.println("  LEER  EL  ESTADO  DE  LAS  LUCES  ");
    Serial.println("------------------------------------");
    delay(1000);
    if (Firebase.getBool(firebaseData, path + "/foco1" ))
    {
      if  (firebaseData.dataType() == "boolean") 
      {
        Serial.print("Foco 1 ");
        Serial.println(firebaseData.boolData() == 1 ? "true" : "false");
        bool valor = firebaseData.boolData();
        digitalWrite(pinLedA, !valor);
        digitalWrite(pinReleA, valor);    
      }  
    }
    if (Firebase.getBool(firebaseData, path + "/foco2" ))
    {
      if  (firebaseData.dataType() == "boolean") 
      {
        Serial.print("Foco 2 ");
        Serial.println(firebaseData.boolData() == 1 ? "true" : "false");
        bool valor = firebaseData.boolData();
        digitalWrite(pinLedB, !valor);
        digitalWrite(pinReleB, valor);    
      }  
    }
    
  }
}

void AvisoInicioPrograma()
{
  digitalWrite(pinLedA, LOW);
  digitalWrite(pinLedB, LOW);

  digitalWrite(pinLedA, HIGH);
  delay(500);
  digitalWrite(pinLedB, HIGH);
  delay(500);

  for (byte i = 0; i <= 2; i++)
  {
    digitalWrite(pinLedA, LOW);
    digitalWrite(pinLedB, LOW);
    delay(250);
    digitalWrite(pinLedA, HIGH);
    digitalWrite(pinLedB, HIGH);
    delay(250);
  }
  digitalWrite(pinLedA, LOW);
  digitalWrite(pinLedB, LOW);
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
