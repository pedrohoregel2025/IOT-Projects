#include "../../RelojOrizabaRTC/ClockPolicy.h"
#include "../../RelojOrizabaRTC/MatrixAnimations.h"
#include "../../RelojOrizabaRTC/LocalControl.h"
#include "../../RelojOrizabaRTC/ManualSelection.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace Reloj;
 const int64_t hour=(1800000000LL/3600)*3600;
 CalendarSchedule s;s.tick(0);assert(!s.take());s.tick(hour);assert(s.take()==1);assert(s.take()==4);assert(!s.take());
 s.tick(hour+15);assert(!s.take());s.tick(hour+300);assert(s.take()==4);assert(!s.take());
 s.tick(hour+600);assert(s.take()==2);assert(s.take()==4);assert(!s.take());
 s.tick(hour+500);s.tick(hour+601);assert(!s.take());
 CalendarSchedule day;int counts[3]={};
 for(int sec=0;sec<86400;++sec){day.tick(hour+sec);uint8_t e;while((e=day.take()))++counts[e==1?0:e==2?1:2];}
 assert(counts[0]==24&&counts[1]==120&&counts[2]==288);
 assert(selectSource(true,true,true,true)==Internet);
 assert(selectSource(true,false,true,true)==RTC);
 assert(selectSource(false,false,true,false)==RTC);
 assert(selectSource(false,false,false,false)==None);
 assert(selectSource(true,false,false,true)==Internal);
 assert(!validEpoch(0)&&validEpoch(hour)&&!validEpoch(4102444800LL));
 for(uint8_t kind=0;kind<5;++kind){
  struct {uint8_t before=0x55;uint8_t pixels[32];uint8_t after=0xaa;} f;
  for(uint16_t step=0;step<MatrixAnimations::length(kind);++step){
   MatrixAnimations::render(kind,step,f.pixels);assert(f.before==0x55&&f.after==0xaa);
   bool lit=false;for(uint8_t col:f.pixels)lit|=col!=0;assert(lit);
  }
 }
 Michi::Button b;b.sample(false,0);b.sample(false,40);b.sample(true,50);assert(b.sample(true,90));assert(!b.sample(true,2000));
 Reloj::ManualSelection selection;
 assert(selection.mode==0&&selection.allowsAutomatic(0));
 selection.next(100);assert(selection.mode==1&&selection.feedback(100));
 assert(!selection.feedback(1300)&&!selection.allowsAutomatic(1300));
 assert(selection.allowsAutomatic(15100));
 selection.next(16000);assert(selection.mode==2);
 selection.next(17000);assert(selection.mode==3);
 selection.next(18000);assert(selection.mode==0);
 selection.next(0xfffffff0);assert(selection.feedback(0x10));
 assert(!selection.allowsAutomatic(0x10));assert(selection.allowsAutomatic(0x4000));
 std::cout<<"24 horas: 24 calendarios, 120 secuencias T/H, 288 animaciones. Prioridad NTP/RTC, limites de cinco efectos y boton: OK\n";
}
