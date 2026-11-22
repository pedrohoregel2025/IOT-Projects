'use strict';
const test = require('node:test');
const assert = require('node:assert/strict');
const core = require('../public/js/app-core.js');
function harness(options = {}) {
  const callbacks = {}, errors = {}, writes = [], updates = [], shown = {}, gauges = {}, messages = [];
  let connected, removed = 0, cleared = 0;
  const store = {
    watch(path, callback, error) { callbacks[path] = callback; errors[path] = error; return () => removed++; },
    write(path, value) { writes.push([path, value]); return options.write ? options.write(path, value) : Promise.resolve(); },
    update(path, value) { updates.push([path, value]); return Promise.resolve(); }
  };
  const view = { device(id, state) { shown[id] = state; }, reading(id, state) { gauges[id] = state; },
    movement() {}, connection(value) { connected = value; }, updated() {}, clock() {}, error(value) { messages.push(value); } };
  const controller = core.createController(store, view, {syncClock: !!options.syncClock, readOnly: !!options.readOnly,
    now: () => new Date(2026, 9, 23, 17, 42),
    timers: {setInterval() { return 1; }, clearInterval() { cleared++; }} });
  controller.start();
  return { controller, callbacks, errors, writes, updates, shown, gauges, messages,
    removed: () => removed, cleared: () => cleared, connected: () => connected };
}
function ready(h) {
  core.devices.forEach(id => h.callbacks['casa1/' + id](false));
  h.callbacks['.info/connected'](true);
}
test('cada foco escribe únicamente su nodo booleano', async () => {
  for (const id of core.devices) {
    const h = harness(); ready(h);
    assert.equal(await h.controller.toggle(id, true), true);
    assert.deepEqual(h.writes, [['casa1/' + id, true]]);
    assert.equal(h.shown[id].checked, true);
  }
});
test('sin conexión o sin lectura válida no se envían órdenes', async () => {
  const h = harness();
  assert.equal(await h.controller.toggle('foco1', true), false);
  h.callbacks['.info/connected'](true);
  for (const invalid of [null, 'on', 1, undefined]) {
    h.callbacks['casa1/foco1'](invalid);
    assert.equal(await h.controller.toggle('foco1', true), false);
    assert.equal(h.shown.foco1.disabled, true);
  }
  assert.deepEqual(h.writes, []);
});
test('la desconexión deshabilita todos los controles y la reconexión los recupera', () => {
  const h = harness(); ready(h);
  assert.equal(h.shown.foco3.disabled, false);
  h.callbacks['.info/connected'](false);
  for (const id of core.devices) assert.equal(h.shown[id].disabled, true);
  h.callbacks['.info/connected'](true);
  for (const id of core.devices) assert.equal(h.shown[id].disabled, false);
});
test('una escritura denegada restaura el estado y muestra error', async () => {
  const h = harness({write: () => Promise.reject(new Error('PERMISSION_DENIED'))}); ready(h);
  assert.equal(await h.controller.toggle('foco3', true), false);
  assert.equal(h.shown.foco3.checked, false);
  assert.equal(h.shown.foco3.disabled, false);
  assert.match(h.messages.at(-1), /No se pudo guardar/);
});
test('un clic repetido durante una escritura no envía otra orden', async () => {
  let finish;
  const h = harness({write: () => new Promise(resolve => finish = resolve)}); ready(h);
  const pending = h.controller.toggle('foco1', true);
  assert.equal(h.shown.foco1.disabled, true);
  assert.equal(await h.controller.toggle('foco1', false), false);
  finish(); await pending;
  assert.equal(h.writes.length, 1);
});
test('fallos de lectura aíslan el control hasta recibir datos válidos', async () => {
  const h = harness(); ready(h); h.errors['casa1/foco2']();
  assert.equal(h.shown.foco2.disabled, true);
  assert.equal(h.shown.foco1.disabled, false);
  assert.equal(await h.controller.toggle('foco2', true), false);
  h.callbacks['casa1/foco2'](true);
  assert.equal(h.shown.foco2.disabled, false);
});
test('lecturas decimales y cero se muestran; datos ausentes no se convierten en cero', () => {
  const h = harness(); ready(h);
  h.callbacks['Farm/Orizaba130/Temperatura'](24.5);
  assert.equal(h.gauges.ragrados.text, '24.5');
  h.callbacks['Farm/Orizaba130/Temperatura'](0);
  assert.equal(h.gauges.ragrados.text, '0');
  h.callbacks['Farm/Orizaba130/Temperatura'](null);
  assert.equal(h.gauges.ragrados.text, '—');
  h.callbacks['Farm/RigRojo/Humedad']('64.3');
  assert.equal(h.gauges.rrhumedad.text, '64.3');
  for (const value of ['', '   ', NaN, Infinity, {}, true, 101, -1]) {
    assert.equal(core.numberReading(value, true), null);
  }
});
test('el reloj mantiene timer/minutets y solo sincroniza conectado', async () => {
  const h = harness({syncClock: true});
  assert.equal(h.updates.length, 0);
  ready(h); await new Promise(setImmediate);
  assert.deepEqual(h.updates, [['timer', {day: 23, month: 10, year: 2026, hours: 17, minutets: 42}]]);
  assert.deepEqual(h.writes, []);
  h.controller.dispose(); assert.equal(h.cleared(), 1);
});
test('iniciar lecturas sin reloj no escribe y dispose libera todos los listeners', () => {
  const h = harness(); ready(h);
  assert.deepEqual(h.writes, []); assert.deepEqual(h.updates, []);
  h.controller.dispose(); assert.equal(h.removed(), 10);
});
test('localhost y demo=1 seleccionan datos simulados sin inicializar Firebase', () => {
  for (const hostname of ['localhost', '127.0.0.1', '[::1]']) {
    assert.equal(core.isDemoLocation({hostname, protocol:'http:', search:'?live=1'}), true);
  }
  assert.equal(core.isDemoLocation({hostname:'appmichiweb.web.app', protocol:'https:', search:'?demo=1'}), true);
  assert.equal(core.isDemoLocation({hostname:'appmichiweb.web.app', protocol:'https:', search:''}), false);
  const values = [], store = core.createDemoStore();
  store.watch('casa1/foco3', v => values.push(v)); store.write('casa1/foco3', true);
  assert.deepEqual(values, [false, true]);
});

test('solo lectura impide escrituras de focos y del reloj incluso conectado', async () => {
  const h = harness({syncClock: true, readOnly: true}); ready(h);
  await new Promise(setImmediate);
  assert.equal(await h.controller.toggle('foco1', true), false);
  assert.equal(h.shown.foco1.disabled, true);
  assert.deepEqual(h.writes, []); assert.deepEqual(h.updates, []);
});

test('se distingue la espera inicial de un nodo que no existe', () => {
  const h = harness(); h.callbacks['.info/connected'](true);
  assert.equal(h.shown.foco1.message, 'Esperando datos');
  assert.equal(h.gauges.ragrados.message, 'Esperando datos');
  h.callbacks['casa1/foco1'](null);
  h.callbacks['Farm/Orizaba130/Temperatura'](null);
  assert.equal(h.shown.foco1.message, 'Sin datos válidos');
  assert.equal(h.gauges.ragrados.message, 'Sin datos válidos');
});

function autoRefreshHarness() {
  let time = 0, visible = true, readyToReload = true, reloads = 0, tick, clears = 0;
  const auto = core.createAutoRefresh({ intervalMs: 10000, now: () => time,
    isVisible: () => visible, canRefresh: () => readyToReload, reload: () => reloads++,
    setInterval(fn, ms) { assert.equal(ms, 1000); tick = fn; return 7; },
    clearInterval(id) { assert.equal(id, 7); clears++; } });
  auto.start();
  return { auto, advance(ms) { time += ms; tick(); },
    visible(value) { visible = value; }, ready(value) { readyToReload = value; },
    reloads: () => reloads, clears: () => clears };
}
test('la página se recarga automáticamente cada diez segundos', () => {
  const h = autoRefreshHarness();
  h.advance(9000); assert.equal(h.reloads(), 0);
  h.advance(1000); assert.equal(h.reloads(), 1);
  h.advance(10000); assert.equal(h.reloads(), 2);
});
test('una escritura pendiente aplaza la recarga y se reintenta al terminar', () => {
  const h = autoRefreshHarness(); h.ready(false);
  h.advance(10000); assert.equal(h.reloads(), 0);
  h.ready(true); h.advance(1000); assert.equal(h.reloads(), 1);
});
test('la pestaña oculta no se recarga y al volver recupera la actualización vencida', () => {
  const h = autoRefreshHarness(); h.visible(false);
  h.advance(30000); assert.equal(h.reloads(), 0);
  h.visible(true); h.auto.tick(); assert.equal(h.reloads(), 1);
  h.auto.dispose(); h.advance(10000);
  assert.equal(h.reloads(), 1); assert.equal(h.clears(), 1);
});
test('la recarga espera las lecturas iniciales y las escrituras de los focos', async () => {
  let finish;
  const h = harness({write: () => new Promise(resolve => finish = resolve)}); ready(h);
  assert.equal(h.controller.canRefresh(), false);
  core.readings.forEach(item => h.callbacks[item.path](25));
  h.callbacks['casa1/movimiento'](0);
  assert.equal(h.controller.canRefresh(), true);
  const pending = h.controller.toggle('foco2', true);
  assert.equal(h.controller.canRefresh(), false);
  finish(); await pending;
  assert.equal(h.controller.canRefresh(), true);
  h.controller.dispose(); assert.equal(h.controller.canRefresh(), false);
});

test('los temporizadores nativos conservan su contexto al iniciar reloj y recarga', () => {
  const vm = require('node:vm');
  const fs = require('node:fs');
  const source = fs.readFileSync(require.resolve('../public/js/app-core.js'),'utf8');
  const context = vm.createContext({});
  vm.runInContext(`
    var started=0, stopped=0;
    function setInterval(callback, delay) {
      if(this!==globalThis) throw new TypeError('Illegal invocation');
      started++; return started;
    }
    function clearInterval(id) {
      if(this!==globalThis) throw new TypeError('Illegal invocation');
      stopped++;
    }
  `,context);
  vm.runInContext(source,context);
  vm.runInContext(`
    var controller=MichiCore.createController({watch:function(){return function(){};}}, {
      connection:function(){}, device:function(){}, reading:function(){}, movement:function(){}
    },{syncClock:true});
    controller.start();
    var refresh=MichiCore.createAutoRefresh({intervalMs:10000,isVisible:function(){return true;},
      canRefresh:function(){return true;},reload:function(){}});
    refresh.start(); refresh.dispose(); controller.dispose();
  `,context);
  assert.equal(context.started,2);assert.equal(context.stopped,2);
});
