/* Programacion diaria independiente por grupo de focos. El ejecutor vive en el NodeMCU. */
(function (root, factory) {
  'use strict';
  var api = factory();
  if (typeof module === 'object' && module.exports) module.exports = api;
  else root.MichiSchedule = api;
}(typeof window !== 'undefined' ? window : this, function () {
  'use strict';
  var path = 'casa1/horarioLuces', statusPath = 'casa1/horarioLucesEstado';
  var zone = 'America/Mexico_City';
  function empty() {
    return { schemaVersion: 1, configured: false, enabled: false, timeZone: zone,
      off: { hour: 0, minute: 0 }, on: { hour: 0, minute: 0 }, updatedAt: 0 };
  }
  function validTime(time) {
    return time && Number.isInteger(time.hour) && time.hour >= 0 && time.hour <= 23 &&
      Number.isInteger(time.minute) && time.minute >= 0 && time.minute <= 59;
  }
  function validate(value) {
    if (!value || value.schemaVersion !== 1 || value.timeZone !== zone || typeof value.enabled !== 'boolean' ||
        value.configured !== true || !validTime(value.on) || !validTime(value.off)) {
      throw new Error('Completa hora y minuto de encendido y apagado.');
    }
    if (value.on.hour === value.off.hour && value.on.minute === value.off.minute) {
      throw new Error('El encendido y el apagado deben tener horarios diferentes.');
    }
    return value;
  }
  function deviceReady(status, now) {
    return !!status && status.schemaVersion === 1 && status.ready === true && status.clockSynced === true &&
      typeof status.lastSeen === 'number' && now - status.lastSeen >= -60000 && now - status.lastSeen < 180000;
  }
  function createController(store, view, options) {
    options = options || {};
    var schedulePath = options.path || path, deviceStatusPath = options.statusPath || statusPath;
    var now = options.now || Date.now, current = empty(), status = null, dirty = false, saving = false;
    var online = false, received = false, failed = false, error = '', disposed = false, off = [];
    function render() {
      if (disposed) return;
      var ready = deviceReady(status, now());
      view.state({ schedule: current, dirty: dirty, saving: saving, online: online, readOnly: !!options.readOnly,
        ready: ready, activationAvailable: ready || !!options.allowPendingActivation, received: received && !failed, error: error,
        message: !online ? 'Sin conexión con Firebase.' : failed ? 'No se pudo leer el horario.' : !received ? 'Esperando horario...' :
          !ready ? (options.allowPendingActivation ? (current.enabled ? 'Activación guardada. El horario comenzará a ejecutarse cuando instales y conectes el programa de horarios del dispositivo 1.' : 'Puedes preparar y guardar la activación. Para ejecutar el horario, instala y conecta el programa de horarios del dispositivo 1.') : 'Pendiente de instalar o conectar el programa actualizado del dispositivo. Puedes guardar el horario; aún no se ejecutará.') :
          !current.configured ? 'Elige las horas de encendido y apagado y guarda el horario.' : current.enabled ? 'Horario diario activo en el dispositivo.' : 'Horario guardado. La automatización está desactivada.' });
    }
    function start() {
      render();
      off.push(store.watch(schedulePath, function (value) {
        received = true;
        if (value && (value.schemaVersion !== 1 || value.timeZone !== zone || typeof value.configured !== 'boolean' ||
            typeof value.enabled !== 'boolean' || !validTime(value.on) || !validTime(value.off))) {
          failed = true; error = 'El horario guardado tiene un formato no compatible.'; render(); return;
        }
        current = value || empty(); failed = false; render();
      }, function () { failed = true; render(); }));
      off.push(store.watch(deviceStatusPath, function (value) { status = value; render(); }, function () { status = null; render(); }));
      off.push(store.watch('.info/connected', function (value) { online = value === true; render(); }, function () { online = false; render(); }));
    }
    function edit() { if (!disposed && !saving) { dirty = true; error = ''; render(); } }
    function cancel() { if (!saving) { dirty = false; error = ''; render(); } }
    async function save(value) {
      if (disposed || saving || !online || !received || failed || options.readOnly) return false;
      try {
        validate(value);
        if (value.enabled && !options.allowPendingActivation && !deviceReady(status, now())) throw new Error('Primero instala y conecta el programa actualizado del dispositivo.');
      } catch (exception) { error = exception.message; render(); return false; }
      var payload = { schemaVersion: 1, configured: true, enabled: value.enabled, timeZone: zone,
        off: { hour: value.off.hour, minute: value.off.minute }, on: { hour: value.on.hour, minute: value.on.minute }, updatedAt: now() };
      saving = true; error = ''; render();
      try {
        await store.write(schedulePath, payload);
        if (!disposed) { current = payload; dirty = false; }
        return true;
      } catch (exception) { if (!disposed) error = 'No se pudo guardar el horario. Tus cambios siguen en el formulario.'; return false; }
      finally { saving = false; render(); }
    }
    function dispose() { disposed = true; off.forEach(function (stop) { if (typeof stop === 'function') stop(); }); }
    return { start: start, edit: edit, cancel: cancel, save: save, dispose: dispose,
      canRefresh: function () { return !disposed && !dirty && !saving; } };
  }
  return { path: path, statusPath: statusPath, zone: zone, empty: empty, validate: validate,
    validTime: validTime, deviceReady: deviceReady, createController: createController };
}));
