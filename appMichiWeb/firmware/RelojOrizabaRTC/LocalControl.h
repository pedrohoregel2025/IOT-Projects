#pragma once
#include <stdint.h>
namespace Michi {
struct Button {
 bool raw=false,stable=false,armed=false;uint32_t changed=0;
 bool sample(bool pressed,uint32_t now){
  if(pressed!=raw){raw=pressed;changed=now;}
  if(uint32_t(now-changed)<40)return false;
  if(!pressed)armed=true;
  if(stable==pressed)return false;
  stable=pressed;
  if(pressed&&armed){armed=false;return true;}
  return false;
 }
};
struct LocalControl {
 bool on[2]={false,false};
 uint8_t dirty=0;
 uint32_t revision[2]={0,0};
 bool manual[2]={false,false};
 int64_t manualEvent[2]={0,0};
 void set(uint8_t channel,bool value){on[channel]=value;++revision[channel];dirty|=uint8_t(1<<channel);}
 void toggle(uint8_t channel,int64_t currentEvent){set(channel,!on[channel]);manual[channel]=true;manualEvent[channel]=currentEvent;}
 void automatic(bool value,int64_t event){
  for(uint8_t channel=0;channel<2;++channel){
   if(manual[channel]&&(manualEvent[channel]==0||manualEvent[channel]>=event)){
    if(manualEvent[channel]==0)manualEvent[channel]=event;
    continue;
   }
   manual[channel]=false;set(channel,value);
  }
 }
 void remote(uint8_t channel,bool value,uint32_t captured){
  if(!(dirty&(1<<channel))&&revision[channel]==captured){on[channel]=value;++revision[channel];}
 }
 void acknowledge(uint8_t mask,const uint32_t *captured){
  for(uint8_t channel=0;channel<2;++channel)if((mask&(1<<channel))&&revision[channel]==captured[channel])dirty&=uint8_t(~(1<<channel));
 }
};
}
