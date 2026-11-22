/* appMichiWeb: vista de la aplicación y selección del origen de datos. */
(function () {
  'use strict';
  var core = window.MichiCore, config = window.MICHI_CONFIG;
  var schedules = window.MichiSchedule, scheduleControllers = [];
  var demo = core.isDemoLocation(window.location), controller;
  var readOnly = new URLSearchParams(window.location.search).get('readonly') === '1';
  var gaugeValues = {}, elements = Object.create(null);
  function element(id) { return elements[id] || (elements[id] = document.getElementById(id)); }
  function error(message) {
    element('app-error').textContent = message;
    element('app-error').hidden = !message;
  }
  var view = {
    connection: function (connected) {
      element('connection-status').textContent = demo ? 'Modo de prueba · sin conexión a Firebase' :
        connected ? 'Conectado a Firebase' : 'Sin conexión · controles deshabilitados';
    },
    device: function (id, state) {
      element(id).checked = state.checked; element(id).disabled = state.disabled;
      element(id).setAttribute('aria-busy', String(state.message === 'Guardando…'));
      element(id + '-status').textContent = state.message;
    },
    movement: function (text, detected, online) {
      element('movement-status').textContent = online ? text : 'Sin conexión';
      element('movement-status').classList.toggle('detected', detected && online);
    },
    reading: function (id, state) {
      element(id).setAttribute('aria-label', state.label);
      element(id + '-status').textContent = state.message;
      if (gaugeValues[id] !== state.text) {
        element(id + '-value').textContent = state.text;
        element(id + '-arc').setAttribute('stroke-dasharray', state.percent + ' 100');
        gaugeValues[id] = state.text;
      }
    },
    updated: function (date) {
      element('last-update').textContent = 'Última lectura: ' + date.toLocaleTimeString('es-MX');
    },
    clock: function (message) { element('clock-status').textContent = message; },
    error: error
  };
  function setupSchedule(store, prefix, paths) {
    function field(id) { return element(prefix + id); }
    var scheduleView = {
      state: function (state) {
        var disabled = !state.online || state.readOnly || state.saving || !state.received;
        ['off', 'on'].forEach(function (key) {
          ['hour', 'minute'].forEach(function (part) {
            var input = field(key + '-' + part);
            input.disabled = disabled;
            if (!state.dirty) input.value = state.schedule.configured ? state.schedule[key][part] : '';
          });
        });
        field('schedule-enabled').disabled = disabled || !state.activationAvailable;
        if (!state.dirty) field('schedule-enabled').checked = state.schedule.enabled;
        field('schedule-save').disabled = disabled || !state.dirty;
        field('schedule-save').textContent = state.saving ? 'Guardando…' : 'Guardar horario';
        field('schedule-cancel').disabled = disabled || !state.dirty;
        field('schedule-status').textContent = state.message;
        field('schedule-error').textContent = state.error;
        field('schedule-error').hidden = !state.error;
        field('schedule-editing').hidden = !state.dirty;
      }
    };
    var scheduleController = schedules.createController(store, scheduleView, {
      readOnly: readOnly, path: paths.path, statusPath: paths.statusPath, allowPendingActivation: !!paths.allowPendingActivation
    });
    ['off-hour', 'off-minute', 'on-hour', 'on-minute', 'schedule-enabled'].forEach(function (id) {
      field(id).addEventListener('change', scheduleController.edit);
      field(id).addEventListener('input', scheduleController.edit);
    });
    field('schedule-cancel').addEventListener('click', scheduleController.cancel);
    field('schedule-form').addEventListener('submit', function (event) {
      event.preventDefault();
      scheduleController.save({ schemaVersion: 1, configured: true, enabled: field('schedule-enabled').checked,
        timeZone: schedules.zone,
        off: { hour: parseInt(field('off-hour').value, 10), minute: parseInt(field('off-minute').value, 10) },
        on: { hour: parseInt(field('on-hour').value, 10), minute: parseInt(field('on-minute').value, 10) } });
    });
    scheduleControllers.push(scheduleController);
    scheduleController.start();
  }
  element('demo-banner').hidden = !demo && !readOnly;
  if (!demo && readOnly) element('demo-banner').textContent = 'Solo lectura · los controles, el horario y el reloj no escriben en Firebase.';
  core.devices.forEach(function (id) {
    element(id).addEventListener('change', function () { if (controller) controller.toggle(id, this.checked); });
  });
  try {
    var store = demo ? core.createDemoStore() : core.createFirebaseStore(window.firebase, config.firebase);
    controller = core.createController(store, view, {
      syncClock: !demo && config.syncClock,
      readOnly: readOnly,
      clockIntervalMs: config.clockIntervalMs
    });
    controller.start();
    setupSchedule(store, '', { path: schedules.path, statusPath: schedules.statusPath, allowPendingActivation: true });
    setupSchedule(store, 'group3-', { path: 'casa1/horarioLuces34', statusPath: 'casa1/horarioLuces34Estado' });
    var brightness = window.MichiBrightness.createController(store, function (state) {
      element('matrix-brightness').value = state.value;
      element('matrix-brightness').disabled = state.disabled;
      element('matrix-brightness-value').textContent = state.value + ' / 15';
      element('matrix-brightness-save').disabled = !state.canSave;
      element('matrix-brightness-status').textContent = state.message;
    }, { readOnly: readOnly });
    element('matrix-brightness').addEventListener('input', function () { brightness.edit(Number(this.value)); });
    element('matrix-brightness-form').addEventListener('submit', function (event) { event.preventDefault(); brightness.save(); });
    brightness.start();
    var brightnessTimer = window.setInterval(brightness.refresh, 15000);
    var autoRefresh;
    if (!demo && config.autoRefreshMs > 0) {
      autoRefresh = core.createAutoRefresh({
        intervalMs: config.autoRefreshMs,
        isVisible: function () { return !document.hidden; },
        canRefresh: function () { return !document.querySelector('select:focus, #matrix-brightness:focus') && brightness.canRefresh() && controller.canRefresh() && scheduleControllers.every(function (schedule) { return schedule.canRefresh(); }); },
        reload: function () { window.location.reload(); }
      });
      autoRefresh.start();
      document.addEventListener('visibilitychange', autoRefresh.tick);
      element('refresh-status').textContent = 'Actualización automática: cada ' + (config.autoRefreshMs === 60000 ? 'minuto' : (config.autoRefreshMs / 1000) + ' segundos');
    }
    if (demo) view.clock('Los cambios de prueba se reinician al recargar');
    window.addEventListener('beforeunload', function () {
      if (autoRefresh) {
        autoRefresh.dispose();
        document.removeEventListener('visibilitychange', autoRefresh.tick);
      }
      scheduleControllers.forEach(function (schedule) { schedule.dispose(); });
      brightness.dispose();
      window.clearInterval(brightnessTimer);
      controller.dispose();
    });
  } catch (exception) {
    console.error('Inicio appMichiWeb:', exception);
    error('No se pudo iniciar la aplicación. Recarga la página para volver a intentarlo.');
    view.connection(false);
  }
}());
