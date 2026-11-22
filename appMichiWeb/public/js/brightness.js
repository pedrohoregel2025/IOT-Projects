/* Brillo MAX7219: 0 es minimo encendido, 15 maximo. */
(function (root) {
  'use strict';
  var path = 'dispositivos/Orizaba130/matriz';
  function valid(value) { return Number.isInteger(value) && value >= 0 && value <= 15; }
  function createController(store, view, options) {
    options = options || {};
    var online = false, received = false, dirty = false, saving = false, failed = false, disposed = false;
    var value = 0, saved = null, ack = null, message = '', unsub = [];
    function render() {
      var recent = ack && Number.isFinite(ack.actualizadoEn) && Date.now() - ack.actualizadoEn >= -60000 && Date.now() - ack.actualizadoEn < 90000;
      view({ value: value, disabled: !online || !received || failed || saving || options.readOnly,
        canSave: online && received && !failed && dirty && !saving && !options.readOnly,
        message: message || (saving ? 'Guardando…' : dirty ? 'Cambio sin guardar' : !online ? 'Sin conexión' :
          recent && valid(ack.brillo) && ack.brillo === saved ? 'Brillo confirmado por el reloj: ' + ack.brillo + '/15' :
          'Guardado en Firebase; esperando confirmación del reloj. Puede requerir actualizar su programa.') });
    }
    function start() {
      unsub.push(store.watch('.info/connected', function (v) { online = v === true; render(); }));
      unsub.push(store.watch(path + '/brillo', function (v) {
        received = true; failed = v !== null && !valid(v); saved = valid(v) ? v : null;
        if (!dirty && !saving) value = saved === null ? 0 : saved;
        message = failed ? 'Configuración de brillo no válida.' : ''; render();
      }, function () { failed = true; message = 'No se pudo leer el brillo de Firebase.'; render(); }));
      unsub.push(store.watch(path + '/estado', function (v) { ack = v; render(); }, function () { ack = null; render(); }));
    }
    function edit(v) { if (saving || options.readOnly || !online || !received || failed || !valid(v)) return; value = v; dirty = saved !== v; message = ''; render(); }
    function save() {
      if (disposed || saving || options.readOnly || !online || !received || failed || !dirty || !valid(value)) return Promise.resolve(false);
      var requested = value; saving = true; message = ''; render();
      return Promise.resolve().then(function () { return store.write(path + '/brillo', requested); }).then(function () {
        if (disposed) return false;
        saving = false; dirty = false; value = saved === null ? requested : saved; render(); return true;
      }, function () { if (!disposed) { saving = false; dirty = true; message = 'No se pudo guardar. Vuelve a intentarlo.'; render(); } return false; });
    }
    return { start: start, edit: edit, save: save, canRefresh: function () { return !dirty && !saving; },
      refresh: render, dispose: function () { disposed = true; unsub.forEach(function (fn) { fn(); }); } };
  }
  var api = { valid: valid, path: path, createController: createController };
  if (typeof module === 'object' && module.exports) module.exports = api; else root.MichiBrightness = api;
}(typeof window !== 'undefined' ? window : this));
