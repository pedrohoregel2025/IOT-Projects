#pragma once
#include <math.h>
namespace Michi {
struct SensorReading {
 bool valid=false,pending=false;
 float temperature=0,humidity=0;
 void update(float t,float h){
  valid=isfinite(t)&&isfinite(h)&&t>=0&&t<=50&&h>=0&&h<=100;
  if(valid){temperature=t;humidity=h;}
  pending=true;
 }
};
}
