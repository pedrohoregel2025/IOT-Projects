#pragma once
#include <stdint.h>
#include <string.h>
namespace MatrixAnimations {
inline void point(uint8_t *frame,int x,int y){if(x>=0&&x<32&&y>=0&&y<8)frame[x]|=uint8_t(1U<<y);}
inline uint16_t length(uint8_t kind){const uint16_t sizes[]={64,96,256,120,100};return sizes[kind%5];}
inline void render(uint8_t kind,uint16_t step,uint8_t *frame){
 memset(frame,0,32);
 switch(kind%5){
 case 0:{ // Barrido de ida y vuelta.
  int x=step%64;if(x>=32)x=63-x;frame[x]=0xff;break;
 }
 case 1:{ // Anillos concentricos simultaneos en los cuatro modulos.
  int ring=(step/6)%8;if(ring>3)ring=7-ring;
  for(int module=0;module<4;++module){for(int i=ring;i<8-ring;++i){
   point(frame,module*8+i,ring);point(frame,module*8+i,7-ring);
   point(frame,module*8+ring,i);point(frame,module*8+7-ring,i);
  }}
  break;
 }
 case 2:{ // Espiral progresiva sobre toda la superficie.
  int left=0,right=31,top=0,bottom=7,count=0;
  while(left<=right&&top<=bottom){
   for(int x=left;x<=right;++x){if(count++<=step)point(frame,x,top);}
   ++top;
   for(int y=top;y<=bottom;++y){if(count++<=step)point(frame,right,y);}
   --right;
   if(top<=bottom){for(int x=right;x>=left;--x)if(count++<=step)point(frame,x,bottom);--bottom;}
   if(left<=right){for(int y=bottom;y>=top;--y)if(count++<=step)point(frame,left,y);++left;}
  }break;
 }
 case 3:{ // Pelota rebotando en los bordes.
  int x=step%62;if(x>31)x=62-x;
  int y=step%14;if(y>7)y=14-y;
  point(frame,x,y);break;
 }
 case 4:{ // Quinta opcion: onda que recorre la matriz.
  for(int x=0;x<32;++x){int y=(x+step)%14;if(y>7)y=14-y;point(frame,x,y);}break;
 }
 }
}
}
