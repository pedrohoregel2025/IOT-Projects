'use strict';
const test = require('node:test');
const assert = require('node:assert/strict');
const schedule = require('../public/js/schedule.js');
const NOW = 1793127600000;
function value(enabled = false) {
  return {schemaVersion: 1, configured: true, enabled, timeZone: schedule.zone,
    off: {hour: 7, minute: 30}, on: {hour: 19, minute: 0}};
}
function harness(options = {}) {
  const callbacks = {}, writes = []; let shown;
  const store = {
    watch(path, callback) { callbacks[path] = callback; return () => {}; },
    write(path, data) { writes.push({path, data}); return options.write ? options.write() : Promise.resolve(); }
  };
  const c = schedule.createController(store, {state: s => shown = s}, {readOnly: !!options.readOnly, allowPendingActivation: !!options.allowPendingActivation, now: () => NOW});
  c.start(); callbacks[schedule.path](null); callbacks['.info/connected'](true);
  if (options.ready) callbacks[schedule.statusPath]({schemaVersion: 1, ready: true, clockSynced: true, lastSeen: NOW});
  return {c, callbacks, writes, state: () => shown};
}
test('horario inicial seguro: desactivado y sin configurar', () => {
  const h = harness(); assert.equal(h.state().schedule.enabled, false);
  assert.equal(h.state().schedule.configured, false); assert.equal(h.state().ready, false);
});
test('guarda un solo horario para ambos focos, sin escribir en los actuadores', async () => {
  const h = harness(); h.c.edit();
  assert.equal(await h.c.save(value()), true);
  assert.equal(h.writes.length, 1); assert.equal(h.writes[0].path, 'casa1/horarioLuces');
  assert.equal(h.writes[0].data.off.hour, 7); assert.equal(h.writes[0].data.on.hour, 19);
  assert.equal(h.writes[0].data.enabled, false); assert.equal(h.writes[0].data.updatedAt, NOW);
});
test('solo activa el horario tras recibir confirmacion reciente del dispositivo', async () => {
  const h = harness(); h.c.edit(); assert.equal(await h.c.save(value(true)), false);
  assert.equal(h.writes.length, 0);
  h.callbacks[schedule.statusPath]({schemaVersion: 1, ready: true, clockSynced: true, lastSeen: NOW});
  assert.equal(await h.c.save(value(true)), true);
});
test('un latido vencido, reloj sin sincronizar o formato antiguo no habilitan automatizacion', () => {
  assert.equal(schedule.deviceReady({schemaVersion:1,ready:true,clockSynced:true,lastSeen:NOW-181000},NOW),false);
  assert.equal(schedule.deviceReady({schemaVersion:1,ready:true,clockSynced:false,lastSeen:NOW},NOW),false);
  assert.equal(schedule.deviceReady({schemaVersion:0,ready:true,clockSynced:true,lastSeen:NOW},NOW),false);
});
test('horas iguales o valores fuera de rango se rechazan sin escribir', async () => {
  const h = harness({ready:true}); h.c.edit();
  for (const change of [v => v.off.hour=24, v => v.on.minute=60, v => v.on.hour=NaN,
    v => v.off={...v.on}, v => v.timeZone='UTC']) {
    const v = value(); change(v); assert.equal(await h.c.save(v), false);
  }
  assert.equal(h.writes.length,0);
});
test('la recarga queda en pausa al editar y se recupera al guardar o descartar', async () => {
  const h=harness(); assert.equal(h.c.canRefresh(),true); h.c.edit(); assert.equal(h.c.canRefresh(),false);
  h.c.cancel(); assert.equal(h.c.canRefresh(),true); h.c.edit(); await h.c.save(value());
  assert.equal(h.c.canRefresh(),true);
});
test('un fallo de escritura conserva el borrador y mantiene pausada la recarga', async () => {
  const h=harness({write:()=>Promise.reject(new Error('denied'))}); h.c.edit();
  assert.equal(await h.c.save(value()),false); assert.equal(h.state().dirty,true);
  assert.equal(h.c.canRefresh(),false); assert.match(h.state().error,/No se pudo guardar/);
});
test('solo lectura y desconexion impiden guardar el horario', async () => {
  const h=harness({readOnly:true,ready:true}); h.c.edit(); assert.equal(await h.c.save(value()),false);
  assert.equal(h.writes.length,0);
  const other=harness();other.callbacks['.info/connected'](false);
  assert.equal(await other.c.save(value()),false);assert.equal(other.writes.length,0);
});
test('una actualizacion remota conserva el borrador hasta que se descarte', () => {
  const h=harness();h.c.edit();h.callbacks[schedule.path](value());
  assert.equal(h.state().dirty,true);assert.equal(h.c.canRefresh(),false);
  h.c.cancel();assert.equal(h.state().schedule.on.hour,19);
});
test('un formato mal formado en Firebase no rompe el formulario ni se sobrescribe', async () => {
  const h=harness();h.callbacks[schedule.path]({enabled:true});
  assert.equal(h.state().received,false);assert.equal(await h.c.save(value()),false);
  assert.equal(h.writes.length,0);
});

test('secciones 1 y 3 guardan horarios y disponibilidad independientes', async () => {
  const watchers = {}, writes = []; const states = {};
  const store = {
    watch(path, callback) { (watchers[path] ||= []).push(callback); return () => {}; },
    write(path, data) { writes.push({path, data}); return Promise.resolve(); }
  };
  function emit(path, value) { watchers[path].forEach(cb => cb(value)); }
  const a = schedule.createController(store, {state: s => states.a = s}, {now: () => NOW});
  const b = schedule.createController(store, {state: s => states.b = s}, {
    now: () => NOW, path: 'casa1/horarioLuces34', statusPath: 'casa1/horarioLuces34Estado'
  });
  a.start(); b.start(); emit(schedule.path, null); emit('casa1/horarioLuces34', null);
  emit('.info/connected', true);
  emit(schedule.statusPath, {schemaVersion:1,ready:true,clockSynced:true,lastSeen:NOW});
  assert.equal(states.a.ready,true); assert.equal(states.b.ready,false);
  b.edit(); assert.equal(a.canRefresh(),true); assert.equal(b.canRefresh(),false);
  assert.equal(await b.save(value(true)),false);
  emit('casa1/horarioLuces34Estado', {schemaVersion:1,ready:true,clockSynced:true,lastSeen:NOW});
  const other=value(true);other.on.hour=21;other.off.hour=9;
  assert.equal(await b.save(other),true);
  a.edit(); assert.equal(await a.save(value(true)),true);
  assert.deepEqual(writes.map(w=>[w.path,w.data.on.hour,w.data.off.hour]),[
    ['casa1/horarioLuces34',21,9],['casa1/horarioLuces',19,7]
  ]);
  emit('casa1/horarioLuces34Estado',null);
  assert.equal(states.a.ready,true);assert.equal(states.b.ready,false);
});

test('seccion 1 puede preparar activacion pendiente sin escribir estados de focos', async () => {
  const h=harness({allowPendingActivation:true});
  assert.equal(h.state().ready,false);assert.equal(h.state().activationAvailable,true);
  h.c.edit();assert.equal(await h.c.save(value(true)),true);
  assert.equal(h.writes.length,1);assert.equal(h.writes[0].path,schedule.path);
  assert.equal(h.writes[0].data.enabled,true);
  assert.match(h.state().message,/Activación guardada/);
  assert.match(h.state().message,/cuando instales y conectes/);
});
test('preparar activacion pendiente conserva validacion y bloqueo en solo lectura', async () => {
  const h=harness({allowPendingActivation:true});h.c.edit();
  const invalid=value(true);invalid.off={...invalid.on};
  assert.equal(await h.c.save(invalid),false);assert.equal(h.writes.length,0);
  const readOnly=harness({allowPendingActivation:true,readOnly:true});
  assert.equal(await readOnly.c.save(value(true)),false);assert.equal(readOnly.writes.length,0);
  h.callbacks['.info/connected'](false);
  assert.equal(await h.c.save(value(true)),false);
});
