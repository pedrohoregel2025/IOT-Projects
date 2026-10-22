//............................................................................
//..Es obligatorio seguir el orden indicado en la inclusion de las librerias..
//............................................................................
//............................   ESP 8266   ..................................
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
//............................    ESP 32    ..................................
//#include <WiFi.h>
//#include <FirebaseESP32.h>

#define FIREBASE_HOST "casa01-fc56a-default-rtdb.firebaseio.com" //Sin http:// o https:// 
#define FIREBASE_AUTH "ArH6EQVGtVofNdEHmCWoCwaxdcbGrY0NRkhUYKCL"

#define WIFI_SSID "Totalplay-77A5"
#define WIFI_PASSWORD "274326638"

String path = "/CasaCuidadela";
//Define un objeto de Firebase
FirebaseData firebaseData;

void printResult(FirebaseData &data);
void CausaError(void);
void InforSetLuzSensor(void);
void InforGetLuzSensor(void);

void setup()
{
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

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Establezca el tiempo de espera de lectura de la base de datos en 1 minuto (máximo 15 minutos)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  
  //Tamaño y  tiempo de espera de escritura, tiny (1s), small (10s), medium (30s) and large (60s).
  //tiny, small, medium, large and unlimited.
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  
  Serial.println("------------------------------------");
  Serial.println("  ACTUALIZAR EL ESTADO DE LAS LUCES ");
  //Also can use Firebase.set instead of Firebase.setDouble
  if (Firebase.setInt(firebaseData, path + "/Jardin/Luz_1", 0)){InforSetLuzSensor();}else{CausaError();}
  if (Firebase.setInt(firebaseData, path + "/Jardin/Luz_2", 1)){InforSetLuzSensor();}else{CausaError();}
  if (Firebase.setInt(firebaseData, path + "/Jardin/Luz_3", 1)){InforSetLuzSensor();}else{CausaError();}

  Serial.println("------------------------------------------");
  Serial.println("ACTUALIZAR INGRESOS Y SALIDAS DEL PERSONAL");
  if (Firebase.setInt(firebaseData, path + "/Seguridad/Ingresos", 100)){InforSetLuzSensor();}else{CausaError();}
  if (Firebase.setInt(firebaseData, path + "/Seguridad/Salida", 80)){InforSetLuzSensor();}else{CausaError();}

  Serial.println("------------------------------------");
  Serial.println("ACTUALIZAR EL ESTADO DE LOS SENSORES");
  
  int NumAleatorio = 0;
  float ValorMedidoSensor = 0;
  
  NumAleatorio = random(10, 100); ValorMedidoSensor = NumAleatorio + 0.465;
  if (Firebase.setFloat(firebaseData, path + "/Sensores/Humedad", ValorMedidoSensor)){InforSetLuzSensor();}else{CausaError();}

  NumAleatorio = random(10, 100); ValorMedidoSensor = NumAleatorio + 0.926;
  if (Firebase.setFloat(firebaseData, path + "/Sensores/Temperatura", ValorMedidoSensor)){InforSetLuzSensor();}else{CausaError();}

  NumAleatorio = random(10, 100); ValorMedidoSensor = NumAleatorio + 0.192;
  if (Firebase.setFloat(firebaseData, path + "/Sensores/Presion", ValorMedidoSensor)){InforSetLuzSensor();}else{CausaError();}

  Serial.println("------------------------------------");
  Serial.println("ACTUALIZAR LOS MENSAJE DE ADVERTENCIA");
  if (Firebase.setString(firebaseData, path + "/Mensajes/Prioridad_1", "VALORES FUERA DE RANGO")){InforSetLuzSensor();}else{CausaError();}
  if (Firebase.setString(firebaseData, path + "/Mensajes/Prioridad_2", "PERSONAL NO AUTORIZADO")){InforSetLuzSensor();}else{CausaError();}
  

  Serial.println("------------------------------------");
  Serial.println("  LEER  EL  ESTADO  DE  LAS  LUCES  ");
  if (Firebase.getInt(firebaseData, path + "/Jardin/Luz_1" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getInt(firebaseData, path + "/Jardin/Luz_2" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getInt(firebaseData, path + "/Jardin/Luz_3" )){InforGetLuzSensor(); }else{CausaError(); }

  Serial.println("------------------------------------");
  Serial.println("   LEER EL ESTADO DE LOS SENSORES   ");
  if (Firebase.getFloat(firebaseData, path + "/Sensores/Humedad" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getFloat(firebaseData, path + "/Sensores/Temperatura" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getFloat(firebaseData, path + "/Sensores/Presion" )){InforGetLuzSensor(); }else{CausaError(); }

  Serial.println("------------------------------------");
  Serial.println("      LEER    LOS     MENSAJES      ");
  if (Firebase.getString(firebaseData, path + "/Mensajes/Prioridad_1" )){InforGetLuzSensor(); }else{CausaError(); }
  if (Firebase.getString(firebaseData, path + "/Mensajes/Prioridad_2" )){InforGetLuzSensor(); }else{CausaError(); }

  //get, getInt, getFloat, getDouble, getBool, getString, getJSON, getArray, getBlob, getFile.
  //set, setInt, setFloat, setDouble, setBool, setString, setJSON, setArray, setBlob and setFile.
}

void loop()
{
  delay(1000);
}

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

void CausaError(void)
{
  Serial.println("ERROR");
  Serial.println("RAZON: " + firebaseData.errorReason());
  Serial.println("------------------------------------");
  Serial.println();
}

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
