//*************************************************************************************************************************
//        INCLUDE
//*************************************************************************************************************************
#include <Wire.h>
#include <RTClib.h>

#include <MD_MAX72xx.h>
//#include <SPI.h>

#include "DHT.h"


// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   10  // or SCK
#define DATA_PIN  8  // or MOSI
#define CS_PIN    9  // or SS

// Specific SPI hardware interface
// Arbitrary pins
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// We always wait a bit between updates of the display
#define  DELAYTIME  100  // in milliseconds

RTC_DS3231 RTC;                           // Se crea el objeto RTCDS3231

byte byMin = 0;
byte bySeg = 0;
byte byHoras = 0;
int iTiempoEspera = 0;
byte byModo = 0;

#define DHTPIN 7     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);


// ========== Control routines ===========
//
void resetMatrix(void)
{
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();
}

void scrollText(const char *p)
{
  uint8_t charWidth;
  uint8_t cBuf[8];  // this should be ok for all built-in fonts

  mx.clear();

  while (*p != '\0')
  {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);

    for (uint8_t i = 0; i <= charWidth; i++)	// allow space between characters
    {
      mx.transform(MD_MAX72XX::TSL);
      if (i < charWidth)
        mx.setColumn(0, cBuf[i]);
      delay(DELAYTIME);
    }
  }
}


void columns()
// Demonstrates the use of setColumn()
{
  //  PRINTS("\nCols 0->max");
  mx.clear();

  for (uint8_t col = 0; col < mx.getColumnCount(); col++)
  {
    mx.setColumn(col, 0xff);
    delay(DELAYTIME / MAX_DEVICES);
    mx.setColumn(col, 0x00);
  }
}


void bullseye()
// Demonstrate the use of buffer based repeated patterns
// across all devices.
{
  //  PRINTS("\nBullseye");
  mx.clear();
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  for (uint8_t n = 0; n < 3; n++)
  {
    byte  b = 0xff;
    int   i = 0;

    while (b != 0x00)
    {
      for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
      {
        mx.setRow(j, i, b);
        mx.setColumn(j, i, b);
        mx.setRow(j, ROW_SIZE - 1 - i, b);
        mx.setColumn(j, COL_SIZE - 1 - i, b);
      }
      mx.update();
      delay(3 * DELAYTIME);
      for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
      {
        mx.setRow(j, i, 0);
        mx.setColumn(j, i, 0);
        mx.setRow(j, ROW_SIZE - 1 - i, 0);
        mx.setColumn(j, COL_SIZE - 1 - i, 0);
      }

      bitClear(b, i);
      bitClear(b, 7 - i);
      i++;
    }

    while (b != 0xff)
    {
      for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
      {
        mx.setRow(j, i, b);
        mx.setColumn(j, i, b);
        mx.setRow(j, ROW_SIZE - 1 - i, b);
        mx.setColumn(j, COL_SIZE - 1 - i, b);
      }
      mx.update();
      delay(3 * DELAYTIME);
      for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
      {
        mx.setRow(j, i, 0);
        mx.setColumn(j, i, 0);
        mx.setRow(j, ROW_SIZE - 1 - i, 0);
        mx.setColumn(j, COL_SIZE - 1 - i, 0);
      }

      i--;
      bitSet(b, i);
      bitSet(b, 7 - i);
    }
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}


void spiral()
// setPoint() used to draw a spiral across the whole display
{
  //  PRINTS("\nSpiral in");
  int  rmin = 0, rmax = ROW_SIZE - 1;
  int  cmin = 0, cmax = (COL_SIZE * MAX_DEVICES) - 1;

  mx.clear();
  while ((rmax > rmin) && (cmax > cmin))
  {
    // do row
    for (int i = cmin; i <= cmax; i++)
    {
      mx.setPoint(rmin, i, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    rmin++;

    // do column
    for (uint8_t i = rmin; i <= rmax; i++)
    {
      mx.setPoint(i, cmax, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    cmax--;

    // do row
    for (int i = cmax; i >= cmin; i--)
    {
      mx.setPoint(rmax, i, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    rmax--;

    // do column
    for (uint8_t i = rmax; i >= rmin; i--)
    {
      mx.setPoint(i, cmin, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    cmin++;
  }
}

void bounce()
// Animation of a bouncing ball
{
  const int minC = 0;
  const int maxC = mx.getColumnCount() - 1;
  const int minR = 0;
  const int maxR = ROW_SIZE - 1;

  int  nCounter = 0;

  int  r = 0, c = 2;
  int8_t dR = 1, dC = 1;	// delta row and column

  //  PRINTS("\nBouncing ball");
  mx.clear();

  while (nCounter++ < 200)
  {
    mx.setPoint(r, c, false);
    r += dR;
    c += dC;
    mx.setPoint(r, c, true);
    delay(DELAYTIME / 2);

    if ((r == minR) || (r == maxR))
      dR = -dR;
    if ((c == minC) || (c == maxC))
      dC = -dC;
  }
}


void showCharset(void)
// Run through display of the the entire font characters set
{
  mx.clear();
  mx.update(MD_MAX72XX::OFF);

  for (uint16_t i = 0; i < 256; i++)
  {
    mx.clear(0);
    mx.setChar(COL_SIZE - 1, i);

    if (MAX_DEVICES >= 3)
    {
      char hex[3];

      sprintf(hex, "%02X", i);

      mx.clear(1);
      mx.setChar((2 * COL_SIZE) - 1, hex[1]);
      mx.clear(2);
      mx.setChar((3 * COL_SIZE) - 1, hex[0]);
    }

    mx.update();
    delay(DELAYTIME * 4);
  }
  mx.update(MD_MAX72XX::ON);
}

//========================================================================================================================
//        void CaratulaReloj()
//
//  Este método se encarga de presentar en pantalla el reloj, el calendario y la temperatura. Se utiliza con bCaratulaReloj, para
//  determinar si el usuario está realizando configuraciones (uso del MODO).
//========================================================================================================================

void CaratulaReloj() {
  DateTime now = RTC.now();           // Obtiene la fecha y hora del RTC
  char texto[10];

  byModo = 0;
  //----------------------------------
  //    DEFINICION DE LA HORA
  //----------------------------------
  sprintf(texto, "%02d:%02d", now.hour(), now.minute());   // Formato 00:00
  Serial.print("Hora actual: ");
  Serial.println(texto);

  mx.control(MD_MAX72XX::INTENSITY, 0);
  mx.clear();
  mx.update(MD_MAX72XX::OFF);

  mx.setChar(16, ':');
  mx.setChar((1 * COL_SIZE) - 3, texto[4]);
  mx.setChar((2 * COL_SIZE) - 4, texto[3]);
  mx.setChar((3 * COL_SIZE) - 2, texto[1]);
  mx.setChar((4 * COL_SIZE) - 3, texto[0]);
  mx.update(MD_MAX72XX::ON);
  iTiempoEspera = millis() + 60000;
}

void CaratulaTemperatura()
{
  int iTemp = dht.readTemperature();
  Serial.print("Temperatura: ");
  Serial.println(iTemp);
  char texto[5];

  byModo = 1;
  sprintf(texto, "%d", iTemp);
  Serial.println(texto);

  mx.control(MD_MAX72XX::INTENSITY, 0);
  mx.clear();
  mx.update(MD_MAX72XX::OFF);

  mx.setChar((1 * COL_SIZE) - 3, 'C');
  mx.setChar((2 * COL_SIZE) - 4, 0x90);

  mx.setChar((3 * COL_SIZE) - 2, texto[1]);
  mx.setChar((4 * COL_SIZE) - 3, texto[0]);
  mx.update(MD_MAX72XX::ON);
  delay(10000);
}

void CaratulaHumedad()
{
  int iTemp = dht.readHumidity();
  Serial.print("Humedad: ");
  Serial.println(iTemp);
  char texto[5];

  byModo = 1;
  sprintf(texto, "%d", iTemp);
  Serial.println(texto);

  mx.control(MD_MAX72XX::INTENSITY, 0);
  mx.clear();
  mx.update(MD_MAX72XX::OFF);

  mx.setChar((1 * COL_SIZE) - 3, 'H');
  mx.setChar((2 * COL_SIZE) - 4, ' ');

  mx.setChar((3 * COL_SIZE) - 2, texto[1]);
  mx.setChar((4 * COL_SIZE) - 3, texto[0]);
  mx.update(MD_MAX72XX::ON);
  delay(10000);
}


void CaratulaFecha()
{
  DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
  byModo = 2;

  Serial.print(now.year(), DEC); // A§o
  Serial.print('/');
  Serial.print(now.month(), DEC); // Mes
  Serial.print('/');
  Serial.print(now.day(), DEC); // Dia
  Serial.print(' ');
  Serial.print(now.hour(), DEC); // Horas
  Serial.print(':');
  Serial.print(now.minute(), DEC); // Minutos
  Serial.print(':');
  Serial.print(now.second(), DEC); // Segundos

  int iYear = now.year();
  int iMes = now.month();
  String sMes = "";
  String sDia;
  int iDia = now.day();
  int n_dia;
  int r_dia;
  int n_mes;
  int t_mes;
  int n_anno;
  int d_anno;
  int t_siglo = 6;

  n_anno = (iYear - 2000);
  d_anno = n_anno / 4;
  n_dia = iDia;
  n_mes = iMes;

  switch (n_mes) {
    case 1:
      t_mes = 0;
      sMes = "Enero";
      break;
    case 2:
      t_mes = 3;
      sMes = "Febrero";
      break;
    case 3:
      t_mes = 3;
      sMes = "Marzo";
      break;
    case 4:
      t_mes = 6;
      sMes = "Abril";
      break;
    case 5:
      t_mes = 1;
      sMes = "Mayo";
      break;
    case 6:
      t_mes = 4;
      sMes = "Junio";
      break;
    case 7:
      t_mes = 6;
      sMes = "Julio";
      break;
    case 8:
      t_mes = 2;
      sMes = "Agosto";
      break;
    case 9:
      t_mes = 5;
      sMes = "Septiembre";
      break;
    case 10:
      t_mes = 0;
      sMes = "Octubre";
      break;
    case 11:
      t_mes = 3;
      sMes = "Noviembre";
      break;
    case 12:
      t_mes = 5;
      sMes = "Diciembre";
      break;
  }

  r_dia = n_dia + t_mes + n_anno + d_anno + t_siglo;
  r_dia = r_dia % 7;

  switch (r_dia) {
    case 1:
      sDia = "Lun";
      break;
    case 2:
      sDia = "Mar";
      break;
    case 3:
      sDia = "Mie";
      break;
    case 4:
      sDia = "Jue";
      break;
    case 5:
      sDia = "Vie";
      break;
    case 6:
      sDia = "Sab";
      break;
    case 0:
      sDia = "Dom";
      break;
  }
  String sFechaCompleta = sDia + " " + String(iDia) + " - " + sMes + " - " + String(iYear) + "     ";
  Serial.println(sFechaCompleta);

  char p[sFechaCompleta.length()];

  for (int i = 0; i < sizeof(p); i++)
    p[i] = sFechaCompleta[i];

  scrollText(p);
  Serial.println(p);
}
void setup()
{
  Serial.begin(9600);
  mx.begin();

  RTC.begin();
  dht.begin();
//  RTC.adjust(DateTime(2022, 7, 8, 15, 10, 0)); // Establece la fecha y hora (Comentar una vez establecida la hora)
//  RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora) 

  Serial.println("Inicializa sistema");
  resetMatrix();
  Serial.println("Verificación de leds");
  //spiral();
  columns();
  CaratulaReloj();
}

void loop()
{
  //--------------------------------------------  Se verifica si se ha pulsado el botón de MODO

#if 1
  DateTime now = RTC.now();
  byMin = now.minute();
  bySeg = now.second();

  //  Serial.println(byMin);
  //  Serial.println(bySeg);
  //  Serial.println(iTiempoEspera);
  //  Serial.println(millis());

  if (byMin == 0 && bySeg == 0)
  {
    bullseye();
    CaratulaFecha();
    columns();
    CaratulaReloj();
  }

  if (byMin % 5 == 0 && bySeg < 10)
  {
    columns();
    CaratulaTemperatura();
    CaratulaHumedad();
    CaratulaReloj();
  }

  if (bySeg == 0)
  {
    columns();
    CaratulaReloj();
  }

#endif
}
