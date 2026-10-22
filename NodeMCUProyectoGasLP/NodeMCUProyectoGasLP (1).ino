//------------------------------------------------------------
// PROYECTO NODEMCU
//------------------------------------------------------------

// Detector de gas natural y lep (sensor mq5)
// Aviso por sonido y led
// Conexión por IoT

//------------------------------------------------------------
// PandaGuard - Febrero 2021
//------------------------------------------------------------

//------------------------------------------------------------
//   Librerias
//------------------------------------------------------------
#include <ESP8266WiFi.h>
#include "FirebaseArduino.h"

//------------------------------------------------------------
//   Firebase
//------------------------------------------------------------
#define FIREBASE_HOST "casa01-fc56a-default-rtdb.firebaseio.com" // databaseUrl SIN https://
#define FIREBASE_AUTH "ArH6EQVGtVofNdEHmCWoCwaxdcbGrY0NRkhUYKCL" // el secreto de la base

//------------------------------------------------------------
//   WiFi Credenciales
//------------------------------------------------------------
#define WIFI_SSID "PANDITA"    // nombre de la red wifi
#define WIFI_PASSWORD "274326638"   // contraseña de la red wifi


/*************************************************

   Public Constants

 *************************************************/

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

// change this to make the song slower or faster
int tempo = 250;

/*************************************************

   Public Constants

 *************************************************/

const byte BUZZER = D5;
const byte LEDROJO = D2;
const byte LEDAMA1 = D4;
const byte LEDAMA2 = D3;
const byte LEDVER = D6;
const byte MAX_INICIO = 250;
bool bEstadoLed = true;
int iValorSensor = 0;
int iSumaSensor = 0;
unsigned long lTiempoEsperaFlash = 0;
unsigned long lTiempoEsperaSensor = 0;
byte byCiclos = 0;
String alarma;

int rangoColorSensorMQ2[] = {150, 250, 350, 450};

int melody[] = {

  // Super Mario Bros theme
  // Score available at https://musescore.com/user/2123/scores/2145
  // Theme by Koji Kondo


  NOTE_E5, 8, NOTE_E5, 8, REST, 8, NOTE_E5, 8, REST, 8, NOTE_C5, 8, NOTE_E5, 8, //1
  NOTE_G5, 4, REST, 4, NOTE_G4, 8, REST, 4,

};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;


void setup() {
  Serial.begin(9600); // velocidad
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  pinMode(LEDROJO, OUTPUT);
  pinMode(LEDAMA1, OUTPUT);
  pinMode(LEDAMA2, OUTPUT);
  pinMode(LEDVER, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LEDROJO, HIGH);
    delay(500);
    digitalWrite(LEDROJO, LOW);
  }
  Serial.println();
  Serial.print("conectado !! ");

  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);



  //--------------------------------------------------
  // Aviso de inicio de programa

  Serial.println("-----------------------------");
  Serial.println("SENSOR DE GAS - INICIALIZANDO");
  Serial.println("-----------------------------");
  AvisoInicioPrograma();
  lTiempoEsperaFlash = millis() + 1000;
  lTiempoEsperaSensor = millis() + 1000;
  digitalWrite(LEDVER, HIGH);
}

void loop() {

  if (Firebase.failed()) {
    Serial.print("Error en conexion de firebase:");
    Serial.println(Firebase.error());
    digitalWrite(LEDROJO, HIGH);
    digitalWrite(LEDAMA1, LOW);
    digitalWrite(LEDAMA2, LOW);
    digitalWrite(LEDVER, LOW);
    delay(1000);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Error en conexion de red");
    digitalWrite(LEDROJO, HIGH);
    digitalWrite(LEDAMA1, LOW);
    digitalWrite(LEDAMA2, LOW);
    digitalWrite(LEDVER, LOW);
    delay(500);
    digitalWrite(LEDROJO, LOW);
    delay(500);
  }
  else
  {
    if (millis() > (lTiempoEsperaFlash))
    {
      FlashLedAviso();
      lTiempoEsperaFlash = millis() + 30000; // Destello cada 30 segundos
    }

    if (millis() > lTiempoEsperaSensor)
    {
      if (byCiclos < 5)
      {
        iValorSensor = iValorSensor + analogRead(A0);
        //Serial.println(analogRead(A0));
        byCiclos++;
      }
      else
      {
        iValorSensor = iValorSensor / 5;
        Serial.print("Sensor MQ-2: ");
        Serial.println(iValorSensor);

        if (iValorSensor > rangoColorSensorMQ2[0])
        {
          Serial.println("Alerta Nivel 0");
          digitalWrite(LEDVER, HIGH);
          digitalWrite(LEDAMA1, LOW);
          digitalWrite(LEDAMA2, LOW);
          digitalWrite(LEDROJO, LOW);
          if (iValorSensor > rangoColorSensorMQ2[1])
          {
            Serial.println("Alerta Nivel 1");
            digitalWrite(LEDAMA1, HIGH);
            digitalWrite(LEDAMA2, LOW);
            digitalWrite(LEDROJO, LOW);

            if (iValorSensor > rangoColorSensorMQ2[2])
            {
              Serial.println("Alerta Nivel 2");
              digitalWrite(LEDAMA2, HIGH);
              digitalWrite(LEDROJO, LOW);
              if (iValorSensor > rangoColorSensorMQ2[3])
              {
                Serial.println("Alerta Nivel 3");
                digitalWrite(LEDROJO, HIGH);
                Firebase.setString("/casa1/alarma", "ON");
              }
              else
              {
                digitalWrite(LEDROJO, LOW);
                Firebase.setString("/casa1/alarma", "OFF");
              }
            }
            else
            {
              digitalWrite(LEDAMA2, LOW);
              digitalWrite(LEDROJO, LOW);
            }
          }
          else
          {
            digitalWrite(LEDAMA1, LOW);
            digitalWrite(LEDAMA2, LOW);
            digitalWrite(LEDROJO, LOW);
          }
        }
        else
        {
          digitalWrite(LEDVER, LOW);
          digitalWrite(LEDAMA1, LOW);
          digitalWrite(LEDAMA2, LOW);
          digitalWrite(LEDROJO, LOW);
        }

        byCiclos = 0;
        iValorSensor = 0;
        lTiempoEsperaSensor = millis() + 1000;  // Muestra el resultado cada 1.5 segundos
      }

    }
  }
}


void AvisoInicioPrograma()
{
  digitalWrite(LEDVER, LOW);
  digitalWrite(LEDAMA1, LOW);
  digitalWrite(LEDAMA2, LOW);
  digitalWrite(LEDROJO, LOW);

  digitalWrite(LEDVER, HIGH);
  delay(500);
  digitalWrite(LEDAMA1, HIGH);
  delay(500);
  digitalWrite(LEDAMA2, HIGH);
  delay(500);
  digitalWrite(LEDROJO, HIGH);
  delay(500);

  for (byte i = 0; i <= 2; i++)
  {
    digitalWrite(LEDVER, LOW);
    digitalWrite(LEDAMA1, LOW);
    digitalWrite(LEDAMA2, LOW);
    digitalWrite(LEDROJO, LOW);
    delay(250);
    digitalWrite(LEDVER, HIGH);
    digitalWrite(LEDAMA1, HIGH);
    digitalWrite(LEDAMA2, HIGH);
    digitalWrite(LEDROJO, HIGH);
    delay(250);
  }
  digitalWrite(LEDVER, LOW);
  digitalWrite(LEDAMA1, LOW);
  digitalWrite(LEDAMA2, LOW);
  digitalWrite(LEDROJO, LOW);
  delay(500);

  tone(BUZZER,200,200);
  noTone(BUZZER);
  tone(BUZZER,400,200);
  noTone(BUZZER);
  tone(BUZZER,600,200);
  noTone(BUZZER);
  /*
     for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

      // calculates the duration of each note
      divider = melody[thisNote + 1];
      if (divider > 0) {
        // regular note, just proceed
        noteDuration = (wholenote) / divider;
      } else if (divider < 0) {
        // dotted notes are represented with negative durations!!
        noteDuration = (wholenote) / abs(divider);
        noteDuration *= 1.5; // increases the duration in half for dotted notes
      }

      // we only play the note for 90% of the duration, leaving 10% as a pause
      tone(BUZZER, melody[thisNote], noteDuration * 0.9);

      // Wait for the specief duration before playing the next note.
      delay(noteDuration);

      // stop the waveform generation before the next note.
      noTone(BUZZER);
    }

  */
}

void FlashLedAviso()
{
  for (byte i = 0; i <= 2; i++)
  {
    digitalWrite(LEDVER, HIGH);
    delay(50);
    digitalWrite(LEDVER, LOW);
    delay(50);
  }


}
