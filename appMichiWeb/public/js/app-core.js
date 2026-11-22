/* Lógica independiente de la pantalla: permite probar sin conectar a Firebase. */
(function (root, factory) {
  'use strict';
  var api = factory();
  if (typeof module === 'object' && module.exports) module.exports = api;
  else root.MichiCore = api;
}(typeof window !== 'undefined' ? window : this, function () {
  'use strict';
  var devices = ['foco1', 'foco2', 'foco3', 'foco4'];
  var readings = [
    { id: 'ragrados', path: 'Farm/Orizaba130/Temperatura', unit: '°C', label: 'Orizaba130, temperatura' },
    { id: 'rahumedad', path: 'Farm/Orizaba130/Humedad', unit: '%', label: 'Orizaba130, humedad', humidity: true },
    { id: 'rrgrados', path: 'Farm/RigRojo/Temperatura', unit: '°C', label: 'MichiCasa, temperatura' },
    { id: 'rrhumedad', path: 'Farm/RigRojo/Humedad', unit: '%', label: 'MichiCasa, humedad', humidity: true }
  ];
  function numberReading(value, humidity) {
    if (typeof value !== 'number' && typeof value !== 'string') return null;
    if (typeof value === 'string' && value.trim() === '') return null;
    var number = Number(value);
    if (!Number.isFinite(number) || (humidity && (number < 0 || number > 100))) return null;
    return number;
  }
  function clockFields(date) {
    return { day: date.getDate(), month: date.getMonth() + 1, year: date.getFullYear(),
      hours: date.getHours(), minutets: date.getMinutes() }; // Contrato original del firmware.
  }
  function createController(store, view, options) {
    options = options || {};
    var online = false, disposed = false, unwatch = [], clockTimer = null, clockPending = false;
    var state = {}, sensorState = {}, movement = null, movementError = false, movementReceived = false;
    var now = options.now || function () { return new Date(); };
    // Conservar el receptor nativo de los temporizadores (WebKit lo exige).
    var timers = options.timers || {
      setInterval: function (callback, delay) { return setInterval(callback, delay); },
      clearInterval: function (id) { clearInterval(id); }
    };
    devices.forEach(function (id) { state[id] = { value: null, pending: false, failed: false, received: false }; });
    readings.forEach(function (item) { sensorState[item.id] = { value: null, failed: false, received: false }; });
    function showDevice(id) {
      var current = state[id];
      view.device(id, { checked: current.value === true,
        disabled: !!options.readOnly || !online || typeof current.value !== 'boolean' || current.pending || current.failed,
        message: current.failed ? 'No se pudo leer' : !online ? 'Sin conexión' : current.pending ? 'Guardando…' :
          !current.received ? 'Esperando datos' : current.value === null ? 'Sin datos válidos' : options.readOnly ? 'Solo lectura' : current.value ? 'Encendido' : 'Apagado' });
    }
    function showReading(item) {
      var current = sensorState[item.id], value = current.value;
      view.reading(item.id, { value: value, percent: value === null ? 0 : Math.max(0, Math.min(100, Math.round(value))),
        text: value === null ? '—' : String(Number(value.toFixed(1))),
        label: item.label + ': ' + (value === null ? 'sin datos' : value + ' ' + item.unit),
        message: current.failed ? 'No se pudo leer' : !online ? 'Sin conexión · última lectura' : !current.received ? 'Esperando datos' : value === null ? 'Sin datos válidos' : '' });
    }
    function showMovement() {
      view.movement(movementError ? 'No se pudo leer' : !movementReceived ? 'Esperando datos' : movement === null ? 'Sin datos válidos' :
        movement ? 'Detectado' : 'Sin movimiento', movement === true, online);
    }
    function refresh() {
      view.connection(online);
      devices.forEach(showDevice); readings.forEach(showReading); showMovement();
    }
    function watch(path, value, error) { unwatch.push(store.watch(path, value, error)); }
    function syncClock() {
      if (!online || disposed || clockPending) return;
      clockPending = true;
      Promise.resolve().then(function () { return store.update('timer', clockFields(now())); }).then(function () {
        if (!disposed && online) view.clock('Reloj sincronizado');
      }, function () { if (!disposed) view.clock('No se pudo sincronizar el reloj'); }).finally(function () { clockPending = false; });
    }
    function start() {
      refresh();
      devices.forEach(function (id) {
        watch('casa1/' + id, function (value) {
          if (disposed) return;
          state[id].value = typeof value === 'boolean' ? value : null;
          state[id].failed = false; state[id].received = true; showDevice(id);
          if (state[id].value !== null) view.updated(now());
        }, function () { if (!disposed) { state[id].failed = true; showDevice(id); } });
      });
      readings.forEach(function (item) {
        watch(item.path, function (value) {
          if (disposed) return;
          sensorState[item.id] = { value: numberReading(value, item.humidity), failed: false, received: true };
          showReading(item);
          if (sensorState[item.id].value !== null) view.updated(now());
        }, function () { if (!disposed) { sensorState[item.id].failed = true; showReading(item); } });
      });
      watch('casa1/movimiento', function (value) {
        if (disposed) return;
        movement = [true, 1, '1'].includes(value) ? true : [false, 0, '0'].includes(value) ? false : null;
        movementError = false; movementReceived = true; showMovement(); if (movement !== null) view.updated(now());
      }, function () { if (!disposed) { movementError = true; showMovement(); } });
      watch('.info/connected', function (value) {
        if (disposed) return;
        var wasOnline = online; online = value === true; refresh();
        if (options.syncClock && !options.readOnly && online && !wasOnline) syncClock();
      }, function () { if (!disposed) { online = false; refresh(); } });
      if (options.syncClock && !options.readOnly) clockTimer = timers.setInterval(syncClock, options.clockIntervalMs || 10000);
    }
    async function toggle(id, value) {
      var current = state[id];
      if (disposed || options.readOnly || !current || !online || current.pending || current.failed ||
          typeof current.value !== 'boolean' || typeof value !== 'boolean') {
        if (current && !disposed) showDevice(id);
        return false;
      }
      var previous = current.value;
      current.pending = true; showDevice(id); view.error('');
      try {
        await store.write('casa1/' + id, value);
        if (!disposed) { current.value = value; view.updated(now()); }
        return true;
      } catch (error) {
        if (!disposed) { current.value = previous; view.error('No se pudo guardar el cambio de ' + id + '. Se restauró su estado anterior.'); }
        return false;
      } finally { current.pending = false; if (!disposed) showDevice(id); }
    }
    function dispose() {
      disposed = true;
      unwatch.forEach(function (off) { if (typeof off === 'function') off(); });
      if (clockTimer !== null) timers.clearInterval(clockTimer);
    }
    function canRefresh() {
      return !disposed && online && !clockPending && devices.every(function (id) {
        return !state[id].pending && (state[id].received || state[id].failed);
      }) && readings.every(function (item) {
        return sensorState[item.id].received || sensorState[item.id].failed;
      }) && (movementReceived || movementError);
    }
    return { start: start, toggle: toggle, dispose: dispose, canRefresh: canRefresh };
  }
  function createFirebaseStore(firebase, config) {
    var app = firebase.initializeApp(config), database = app.database();
    return {
      watch: function (path, listener, error) {
        var ref = database.ref(path), onValue = function (snapshot) { listener(snapshot.val()); };
        ref.on('value', onValue, error);
        return function () { ref.off('value', onValue); };
      },
      write: function (path, value) { return database.ref(path).set(value); },
      update: function (path, value) { return database.ref(path).update(value); }
    };
  }
  function createDemoStore() {
    var values = { 'casa1/foco1': false, 'casa1/foco2': true, 'casa1/foco3': false, 'casa1/foco4': false, 'casa1/movimiento': 0,
      'Farm/Orizaba130/Temperatura': 24.5, 'Farm/Orizaba130/Humedad': 55,
      'Farm/RigRojo/Temperatura': 27.2, 'Farm/RigRojo/Humedad': 48, '.info/connected': true };
    values['casa1/horarioLuces'] = { schemaVersion: 1, configured: false, enabled: false,
      timeZone: 'America/Mexico_City', off: { hour: 0, minute: 0 }, on: { hour: 0, minute: 0 }, updatedAt: 0 };
    values['casa1/horarioLucesEstado'] = { schemaVersion: 1, firmwareVersion: '2.0.0', ready: true,
      clockSynced: true, lastSeen: Date.now() };
    values['casa1/horarioLuces34'] = JSON.parse(JSON.stringify(values['casa1/horarioLuces']));
    values['casa1/horarioLuces34Estado'] = Object.assign({}, values['casa1/horarioLucesEstado']);
    var listeners = {};
    return {
      watch: function (path, listener) {
        if (!listeners[path]) listeners[path] = [];
        listeners[path].push(listener); listener(values[path] === undefined ? null : values[path]);
        return function () { listeners[path] = listeners[path].filter(function (item) { return item !== listener; }); };
      },
      write: function (path, value) {
        values[path] = value; (listeners[path] || []).forEach(function (listener) { listener(value); });
        return Promise.resolve();
      },
      update: function () { return Promise.resolve(); }
    };
  }
  // Recarga periódica compatible con la aplicación original. El intervalo de
  // comprobación permite aplazar un segundo si coincide con una escritura.
  function createAutoRefresh(options) {
    var intervalMs = options.intervalMs || 10000;
    var now = options.now || Date.now;
    var schedule = options.setInterval || function (callback, delay) { return setInterval(callback, delay); };
    var cancel = options.clearInterval || function (id) { clearInterval(id); };
    var nextRefresh = now() + intervalMs, timer = null, stopped = false;
    function tick() {
      if (stopped || now() < nextRefresh || !options.isVisible() || !options.canRefresh()) return;
      nextRefresh = now() + intervalMs;
      options.reload();
    }
    return {
      start: function () { if (timer === null && !stopped) timer = schedule(tick, 1000); },
      tick: tick,
      dispose: function () { stopped = true; if (timer !== null) cancel(timer); timer = null; }
    };
  }
  function isDemoLocation(location) {
    return location.protocol === 'file:' || ['localhost', '127.0.0.1', '[::1]', '::1'].includes(location.hostname) ||
      new URLSearchParams(location.search).get('demo') === '1';
  }
  return { createAutoRefresh: createAutoRefresh, createController: createController, createFirebaseStore: createFirebaseStore,
    createDemoStore: createDemoStore, numberReading: numberReading, clockFields: clockFields,
    isDemoLocation: isDemoLocation, readings: readings, devices: devices };
}));
