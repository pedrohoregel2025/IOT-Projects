#include "../NodeMCUProyectoLuz/SensorReading.h"
#include <cassert>
#include <limits>
int main(){
 Michi::SensorReading r;assert(!r.valid&&!r.pending);
 r.update(24.5f,55);assert(r.valid&&r.pending&&r.temperature==24.5f&&r.humidity==55);
 r.pending=false;r.update(NAN,55);assert(!r.valid&&r.pending);
 r.update(25,NAN);assert(!r.valid);
 r.update(std::numeric_limits<float>::infinity(),50);assert(!r.valid);
 r.update(25,101);assert(!r.valid);r.update(25,-1);assert(!r.valid);
 r.update(-1,50);assert(!r.valid);r.update(51,50);assert(!r.valid);
 r.update(0,0);assert(r.valid);r.update(50,100);assert(r.valid);
 r.update(23,60);assert(r.valid&&r.temperature==23&&r.humidity==60);
}
