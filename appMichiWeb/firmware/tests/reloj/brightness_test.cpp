#include "../../RelojOrizabaRTC/Brightness.h"
#include <cassert>
int main(){
 for(int i=0;i<=15;i++){assert(Reloj::validBrightness(i));Reloj::BrightnessMemory memory;memory.set(i);assert(memory.valid());assert(memory.value==i);memory.check^=1;assert(!memory.valid());}
 assert(!Reloj::validBrightness(-1));assert(!Reloj::validBrightness(16));Reloj::BrightnessMemory memory;memory.magic=0xFF;assert(!memory.valid());static_assert(sizeof(memory)==4,"EEPROM reservado");
}
