#pragma once
#include <stdint.h>
namespace Reloj {
struct ManualSelection {
 uint8_t mode=0;bool active=false;uint32_t changedAt=0;
 void next(uint32_t now){mode=(mode+1)%4;active=true;changedAt=now;}
 bool feedback(uint32_t now)const{return active&&uint32_t(now-changedAt)<1200;}
 bool allowsAutomatic(uint32_t now)const{return !active||uint32_t(now-changedAt)>=15000;}
};
}
