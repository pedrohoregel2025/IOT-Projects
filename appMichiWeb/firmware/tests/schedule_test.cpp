#include "../NodeMCUProyectoLuz/DailySchedule.h"
#include <cassert>
#include <iostream>
using namespace Michi;
// Dia arbitrario moderno. La variable localMinutes indica hora de Ciudad de Mexico.
const int64_t DAY = 20753LL * 86400;
int64_t at(int dayOffset, int localMinutes) {
  return DAY + dayOffset * 86400LL + localMinutes * 60 - UTC_OFFSET_SECONDS;
}
int main() {
  DailySchedule night; night.configured = true; night.enabled = true;
  night.onHour = 19; night.offHour = 7;
  assert(night.valid());
  assert(!latestEvent(night, at(0, 18*60+59)).turnOn);
  assert(latestEvent(night, at(0, 19*60)).turnOn);
  assert(latestEvent(night, at(1, 0)).turnOn);
  assert(latestEvent(night, at(1, 6*60+59)).turnOn);
  assert(!latestEvent(night, at(1, 7*60)).turnOn);
  assert(latestEvent(night, at(0, 23*60+59)).eventAt == latestEvent(night, at(1, 0)).eventAt);
  ScheduleEngine engine; Decision decision;
  assert(engine.due(night, at(0, 20*60), decision)); // Arranque: recupera el periodo actual.
  engine.applied(decision);
  assert(!engine.due(night, at(0, 21*60), decision)); // Respeta un cambio manual.
  assert(!engine.due(night, at(1, 0), decision)); // Medianoche no repite una orden.
  assert(engine.due(night, at(1, 7*60+4), decision)); // Recupera una ejecucion atrasada.
  assert(!decision.turnOn); engine.applied(decision);
  assert(!engine.due(night, at(1, 7*60+5), decision));
  assert(engine.due(night, at(1, 19*60), decision));
  DailySchedule day = night; day.onHour = 6; day.offHour = 18;
  assert(!latestEvent(day, at(0, 5*60+59)).turnOn);
  assert(latestEvent(day, at(0, 6*60)).turnOn);
  assert(!latestEvent(day, at(0, 18*60)).turnOn);
  day.onMinute = 15; day.offMinute = 45;
  assert(!latestEvent(day, at(0, 6*60+14)).turnOn);
  assert(latestEvent(day, at(0, 6*60+15)).turnOn);
  day.enabled = false; assert(!engine.due(day, at(0, 10*60), decision));
  day.enabled = true; day.onHour = day.offHour; day.onMinute = day.offMinute;
  assert(!day.valid()); assert(!engine.due(day, at(0, 10*60), decision));
  day = night; day.onHour = 24; assert(!day.valid());
  assert(!engine.due(night, 0, decision)); // Nunca acciona sin hora valida.
  engine.reset(); assert(engine.due(night, at(0, 21*60), decision));
  std::cout << "Pruebas C++ aprobadas: horario diurno, nocturno, medianoche, minutos, arranque, cambios manuales y reloj invalido.\n";
}
