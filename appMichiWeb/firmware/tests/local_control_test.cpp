#include "../NodeMCUProyectoLuz/LocalControl.h"
#include <cassert>
#include <iostream>
using namespace Michi;
int main(){
 Button b;
 assert(!b.sample(false,0));assert(!b.sample(false,40));
 assert(!b.sample(true,50));assert(!b.sample(false,60));assert(!b.sample(true,70));
 assert(!b.sample(true,109));assert(b.sample(true,110));assert(!b.sample(true,5000));
 assert(!b.sample(false,5010));assert(!b.sample(false,5050));
 assert(!b.sample(true,5060));assert(b.sample(true,5100));
 Button boot;assert(!boot.sample(true,0));assert(!boot.sample(true,100));
 assert(!boot.sample(false,200));assert(!boot.sample(false,240));assert(!boot.sample(true,250));assert(boot.sample(true,290));
 Button wrap;wrap.sample(false,0xffffff00);wrap.sample(false,0xffffff40);wrap.sample(true,0xfffffff0);assert(wrap.sample(true,0x20));
 LocalControl c;c.toggle(0,100);assert(c.on[0]&&c.dirty==1&&!c.on[1]);
 uint32_t captured[]={c.revision[0],c.revision[1]};
 c.remote(0,false,captured[0]);assert(c.on[0]); // Lectura obsoleta no revierte boton.
 c.toggle(0,100);c.acknowledge(1,captured);assert(c.dirty==1); // Cambio durante escritura.
 captured[0]=c.revision[0];c.acknowledge(1,captured);assert(c.dirty==0);
 auto old=c.revision[1];c.toggle(1,100);c.remote(1,false,old);assert(c.on[1]);
 c.automatic(false,100);assert(c.on[1]); // Manual hasta siguiente limite.
 c.automatic(false,200);assert(!c.on[0]&&!c.on[1]);
 LocalControl offline;offline.toggle(0,0);offline.automatic(false,500);assert(offline.on[0]);
 offline.automatic(false,600);assert(!offline.on[0]);
 LocalControl remote;remote.remote(1,true,0);assert(remote.on[1]&&remote.dirty==0);
 remote.toggle(0,0);assert(remote.on[1]); // Canales independientes.
 std::cout<<"Botones, rebotes, arranque sin red, sincronizacion concurrente y prioridad manual: OK\n";
}
