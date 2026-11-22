const {test}=require('node:test');const assert=require('node:assert/strict');
const api=require('../public/js/brightness.js');
function harness(options={}){
 const listeners={},writes=[];let state;let reject=false;
 const store={watch(p,fn){listeners[p]=fn;return()=>delete listeners[p];},write(p,v){writes.push([p,v]);if(reject)return Promise.reject(Error('denied'));listeners[p](v);return Promise.resolve();}};
 const c=api.createController(store,s=>state=s,options);c.start();
 return {c,writes,get state(){return state;},emit(p,v){listeners[p](v);},fail(){reject=true;},connect(){listeners['.info/connected'](true);listeners[api.path+'/brillo'](0);}};
}
test('rango entero 0..15, rechaza valores fuera del rango y tipos incorrectos',()=>{
 for(let i=0;i<=15;i++)assert(api.valid(i));for(const v of [-1,16,1.5,null,'2',NaN,Infinity])assert(!api.valid(v));
});
test('guarda cero y quince, sincroniza valor remoto y confirmacion reciente',async()=>{
 const h=harness();h.connect();h.c.edit(15);assert(!h.c.canRefresh());assert(await h.c.save());assert.deepEqual(h.writes,[[api.path+'/brillo',15]]);assert(h.c.canRefresh());
 assert.match(h.state.message,/esperando/);h.emit(api.path+'/estado',{brillo:15,actualizadoEn:Date.now()});assert.match(h.state.message,/confirmado/);
 h.emit(api.path+'/estado',{brillo:15,actualizadoEn:Date.now()-100000});assert.match(h.state.message,/esperando/);
 h.emit(api.path+'/brillo',4);assert.equal(h.state.value,4);h.c.edit(0);await h.c.save();assert.equal(h.writes[1][1],0);
});
test('sin red y solo lectura no escriben; errores conservan edicion',async()=>{
 const a=harness();a.emit(api.path+'/brillo',0);a.c.edit(5);await a.c.save();assert.equal(a.writes.length,0);
 const b=harness({readOnly:true});b.connect();b.c.edit(5);await b.c.save();assert.equal(b.writes.length,0);
 const c=harness();c.connect();c.c.edit(6);c.fail();assert.equal(await c.c.save(),false);assert.equal(c.state.value,6);assert(c.state.canSave);assert(!c.c.canRefresh());
 c.emit(api.path+'/brillo',2);assert.equal(c.state.value,6);c.c.dispose();
});
test('configuracion invalida deshabilita control',()=>{const h=harness();h.connect();h.emit(api.path+'/brillo',30);assert(h.state.disabled);});
