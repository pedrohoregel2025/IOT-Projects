
// Prueba de Reloj Tiempo Real (RTC) y Modulo I2C para LCD 2x16 By: http://dinastiatecnologica.com
// Conexion i2C :Arduino UNO  SCL-> A5   SDA->A4  VCC->5V  GND->GND
// Conexion RTC :Arduino UNO    GND->GND VCC->5V SCL-> SCL  SDA->SDA los dos pines despues del ...12,13,GND,AREF,SDA,SCL
// NOTA: se debe cargar dos veces este programa 1.Con la linea 9= RTC.adjust(DateTime(__DATE__, __TIME__));
//                                              2. Sin esa linea

//*************************************************************************************************************************
//        INCLUDE
//*************************************************************************************************************************
#include <Wire.h> 
#include <RTClib.h>


//*************************************************************************************************************************
//        GLOBAL
//*************************************************************************************************************************

  RTC_DS3231 RTC;                       // Se crea el objeto RTCDS3231

//========================================================================================================================
//        void setup()
//========================================================================================================================

void setup () 
{
  Wire.begin();                        // Inicia el puerto I2C
  Serial.begin(9600);
  Serial.println("Inicializa sistema");
  RTC.begin();                         // Inicia la comunicaci¢n con el RTC

   RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora) 

    // Fijar a fecha y hora específica. En el ejemplo, 21 de Enero de 2016 a las 03:00:00
    // RTC.adjust(DateTime(2014, 1, 21, 3, 0, 0));
                                            //el anterior se usa solo en la configuracion inicial
}

//========================================================================================================================
//        void loop()
//========================================================================================================================

void loop()
{
  DateTime now = RTC.now();           // Obtiene la fecha y hora del RTC
  char texto[10];

  //----------------------------------
  //    DEFINICION DE LA HORA
  //----------------------------------
  sprintf(texto, "%02d:%02d", now.hour(), now.minute());   // Formato 00:00
  Serial.print("Hora actual: ");
  Serial.println(texto);
  delay(1000);
}

//========================================================================================================================
//        END
//========================================================================================================================
