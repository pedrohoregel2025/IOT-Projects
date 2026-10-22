#include <TimeLib.h>      // incluye libreria TimeLib   
#include <TM1637Display.h>    // incluye libreria TM1637

#define CLK 2       // CLK a pin 2
#define DIO 3       // DIO a pin 3

TM1637Display display(CLK, DIO);  // crea objeto

void setup(){
  display.setBrightness(5);   // establece nivel de brillo
  setTime(0,0,0,10,2,2022);    // establece fecha y horario inicial
}

void loop(){
  time_t t = now();     // obtiene fecha y horario
  display.showNumberDecEx(hour(t), 0b01000000, false, 2, 0);  // muestra hora y enciende
                // los dos puntos separadores  
  display.showNumberDec(minute(t), false, 2, 2);    // muestra minutos
  delay(100);       // demora de 100 mseg.
}
