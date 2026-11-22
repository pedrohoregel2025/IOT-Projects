#pragma once
#include <stdint.h>
namespace Reloj {
inline bool validBrightness(int value){return value>=0&&value<=15;}
struct BrightnessMemory {
 uint8_t magic=0xB6,version=1,value=0,check=0xFF;
 bool valid() const {return magic==0xB6&&version==1&&validBrightness(value)&&check==uint8_t(value^0xFF);}
 void set(uint8_t v){magic=0xB6;version=1;value=v;check=v^0xFF;}
};
}
