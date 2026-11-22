#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
namespace Michi {
struct WifiProfile {
 uint32_t magic=0;char ssid[33]={};char password[65]={};uint32_t checksum=0;
 uint32_t hash()const{
  uint32_t value=2166136261UL;
  for(size_t n=0;n<offsetof(WifiProfile,checksum);++n){value^=reinterpret_cast<const uint8_t*>(this)[n];value*=16777619UL;}
  return value;
 }
 bool valid()const{return magic==0x4c435731&&ssid[0]&&memchr(ssid,0,sizeof(ssid))&&memchr(password,0,sizeof(password))&&checksum==hash();}
 bool assign(const char *name,const char *key){
  size_t n=strlen(name),k=strlen(key);
  if(!n||n>32||k>64||(k>0&&k<8))return false;
  memset(this,0,sizeof(*this));magic=0x4c435731;memcpy(ssid,name,n);memcpy(password,key,k);checksum=hash();return true;
 }
};
inline bool offlineLedLevel(bool online,bool relayOn,uint32_t now){return online?!relayOn:((now/500)%2==0);}
}
