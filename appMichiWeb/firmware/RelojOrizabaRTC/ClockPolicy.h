#pragma once
#include <stdint.h>
namespace Reloj {
inline bool validEpoch(int64_t epoch){return epoch>=1704067200LL&&epoch<4102444800LL;}
struct CalendarSchedule {
 int64_t lastMinute=-1;uint8_t pending=0;
 enum Event:uint8_t {Calendar=1,Sensors=2,Animation=4};
 void tick(int64_t epoch){
  if(!validEpoch(epoch))return;
  int64_t minute=epoch/60;
  if(minute<=lastMinute)return;
  lastMinute=minute;
  if(minute%60==0)pending|=Calendar;
  if(minute%10==0&&minute%60!=0)pending|=Sensors;
  if(minute%5==0)pending|=Animation;
 }
 uint8_t take(){for(uint8_t event=1;event<=4;event<<=1)if(pending&event){pending&=uint8_t(~event);return event;}return 0;}
};
enum ClockSource:uint8_t {None,Internet,RTC,Internal};
inline ClockSource selectSource(bool synced,bool online,bool rtcValid,bool internalValid){
 if(synced&&online&&internalValid)return Internet;
 if(rtcValid)return RTC;
 return internalValid?Internal:None;
}
}
