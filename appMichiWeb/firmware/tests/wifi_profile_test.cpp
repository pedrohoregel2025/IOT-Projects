#include "../NodeMCUProyectoLuz/WifiProfile.h"
#include <cassert>
#include <string>
int main(){
 Michi::WifiProfile p;assert(!p.valid());
 assert(p.assign("Casa","12345678")&&p.valid());
 auto copy=p;copy.password[0]^=1;assert(!copy.valid());
 assert(!p.assign("","12345678"));assert(!p.assign("Casa","corta"));
 assert(!p.assign(std::string(33,'x').c_str(),"12345678"));
 assert(!p.assign("Casa",std::string(65,'x').c_str()));
 assert(p.assign(std::string(32,'x').c_str(),std::string(64,'y').c_str())&&p.valid());
 assert(p.assign("Abierta","")&&p.valid());
 for(uint32_t now=0;now<3000;now+=25){
  // Sin Internet los dos canales tienen el mismo nivel, sin importar el rele.
  assert(Michi::offlineLedLevel(false,false,now)==Michi::offlineLedLevel(false,true,now));
  assert(Michi::offlineLedLevel(true,false,now));
  assert(!Michi::offlineLedLevel(true,true,now));
 }
 assert(Michi::offlineLedLevel(false,false,0));
 assert(!Michi::offlineLedLevel(false,false,500));
 assert(Michi::offlineLedLevel(false,false,1000));
}
