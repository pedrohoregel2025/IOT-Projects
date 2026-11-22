#pragma once
#include <Wire.h>
#include <RTClib.h>
#include <coredecls.h>
#include <sys/time.h>
#include "ClockPolicy.h"
class ClockService {
 RTC_DS3231 rtc;
 bool present=false,rtcValid=false,ntpSeen=false,updateRtc=false;
 uint32_t sampledAt=0;int64_t rtcEpoch=0;
 bool responds(){Wire.beginTransmission(0x68);return Wire.endTransmission()==0;}
 void readRtc(){
  if(!responds()){rtcValid=false;return;}
  DateTime value=rtc.now();
  rtcValid=!rtc.lostPower()&&value.isValid()&&Reloj::validEpoch(value.unixtime());
  if(rtcValid){rtcEpoch=value.unixtime();sampledAt=millis();}
 }
public:
 Reloj::ClockSource source=Reloj::None;
 bool rtcHealthy()const{return rtcValid;}
 void begin(){
  Wire.begin(D2,D1);Wire.setClock(100000);Wire.setClockStretchLimit(1500);
  present=rtc.begin(&Wire);
  if(present)readRtc();
  if(rtcValid){timeval value{static_cast<time_t>(rtcEpoch),0};settimeofday(&value,nullptr);}
  settimeofday_cb([this](bool fromNtp){if(fromNtp){ntpSeen=true;updateRtc=true;}});
  configTime(0,0,"pool.ntp.org","time.google.com","time.cloudflare.com");
  Serial.println(rtcValid?F("RTC: hora recuperada (UTC)"):F("RTC: sin hora valida; requiere NTP y pila de respaldo"));
 }
 void tick(){
  if(updateRtc){
   updateRtc=false;int64_t epoch=time(nullptr);
   if(Reloj::validEpoch(epoch)&&responds()){
    if(!present)present=rtc.begin(&Wire);
    if(present){rtc.adjust(DateTime(static_cast<uint32_t>(epoch)));readRtc();Serial.println(F("RTC: actualizado desde NTP"));}
   }
  }
  static uint32_t checked=0;
  if(uint32_t(millis()-checked)>=1000){checked=millis();if(present)readRtc();}
 }
 int64_t utc(bool online){
  const int64_t internal=time(nullptr);
  source=Reloj::selectSource(ntpSeen,online,rtcValid,Reloj::validEpoch(internal));
  if(source==Reloj::RTC)return rtcEpoch+uint32_t(millis()-sampledAt)/1000;
  return source==Reloj::None?0:internal;
 }
};
