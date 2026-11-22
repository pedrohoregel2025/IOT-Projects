# Entrega de repositorio — 22 de noviembre de 2026

- Documentacion consolidada README.MD, perfiles de hardware y pruebas reproducibles.
- Configuracion web y Firebase Hosting con valores de ejemplo.
- Claves de AP de MichiCasa y reloj trasladadas a CONFIG_AP_PASSWORD en secrets.h.
- Incluido reloj Orizaba130 1.0.2 y conservado perfil anterior con reles.
- Sin cambios a Hosting, reglas Firebase ni dispositivos instalados.

# Web 1.3.6 — 3 de noviembre de 2026

- Control de brillo de Orizaba130 ubicado al final de la tercera seccion, despues del horario.

# Web 1.3.5 — 3 de noviembre de 2026

- Brillo remoto 0..15 y confirmacion reciente del reloj, sin modificar lecturas de sensores.

# Firmware LuzCasa 2.3.1 — 31 de octubre de 2026

- La red fija MichiCasa-Config se anuncia desde el arranque y permanece disponible con o sin Internet.
- Retiro del cierre automatico; reintento de inicio del AP cada cinco segundos si falla.
- Se conserva parpadeo conjunto por falta de conectividad, sin alterar los reles.

# Firmware LuzCasa 2.3.0 — 31 de octubre de 2026

- Red de configuracion fija MichiCasa-Config y contrasena fija solicitada por el usuario.
- Portal local para guardar otra red de 2.4 GHz con persistencia y recuperacion ante fallo.
- Comprobacion de acceso a Internet; LED sincronizados en desconexion, sin afectar reles ni botones.

# Web 1.3.4 — 31 de octubre de 2026

- Nombre visible MichiCasa en lugar de Rig Rojo, incluidos indicadores accesibles. Se conserva la ruta Farm/RigRojo para el firmware existente.

# Firmware LuzCasa 2.2.0 — 31 de octubre de 2026

- DHT11 en D0 con el controlador GPIO16 probado en el proyecto 2.
- Lecturas cada 10 segundos, independientes de Internet, publicadas en Farm/RigRojo al conectar.
- Errores del sensor eliminan mediciones antiguas en la web; se conservan botones, LED, reles y horario.

# Firmware LuzCasa 2.1.0 — 31 de octubre de 2026

- Botones y LED independientes de la conexion WiFi/Firebase desde el arranque.
- Antirrebote, una conmutacion por pulsacion y sincronizacion de cambios locales al reconectar.
- Proteccion por canal ante respuestas antiguas y cambios durante solicitudes de red.
- Se conservan pines, polaridad y horario diario. Se incluye instalador local con respaldo previo.
- Compilacion y pruebas C++ aprobadas; instalacion fisica pendiente. Version web sin cambios.

# 1.3.3 — 27 de octubre de 2026

- Recarga automática cada minuto (60000 ms) y etiqueta correspondiente en pantalla.

# 1.3.2 — 27 de octubre de 2026

- Sección 1 permite preparar y guardar la activación de su horario sin esperar el reporte del dispositivo.
- Mensaje explícito de ejecución pendiente hasta instalar/conectar el programa del dispositivo 1.
- Se conservan validación, bloqueos de solo lectura/desconexión y comportamiento independiente de sección 3. 30 pruebas aprobadas.

# 1.3.1 — 27 de octubre de 2026

- Corregido `Illegal invocation` al iniciar el temporizador del reloj en modo normal: desbloquea el inicio de horarios y de recarga.
- Nueva prueba del contexto de temporizadores nativos; 28 pruebas aprobadas.
- Recursos propios con versión en URL y diagnóstico de errores de inicio.

# 1.3.0 — 27 de octubre de 2026

- Diseño adaptable a teléfonos, controles táctiles mayores, navegación entre secciones y horarios con botones de ancho completo.
- Medidores SVG nativos; retiro de cuatro dependencias de presentación de la carga inicial (aproximadamente 250 KB sin comprimir).
- Estilos unificados; se corrigió el área táctil de interruptores para eliminar desbordamientos en pantallas estrechas.
- Se conserva la actualización automática, con pausa adicional mientras un selector de hora tiene el foco.

# 1.2.0 — 27 de octubre de 2026

- Paleta verde claro y Orizaba130 en lugar de Rig Azul.
- Foco 4 y horario exclusivo para Focos 3/4; movimiento conservado.
- Controladores y borradores independientes; la disponibilidad del dispositivo 1 no habilita el dispositivo 2.
- Rutas nuevas inicializadas sin sobreescribir datos anteriores ni fabricar lecturas.
- Firmware Orizaba130 preparado y compilado, pendiente de instalación.

# Cambios

## 1.1.0 — 27 de octubre de 2026

Tres secciones, paleta verde y horario diario compartido de Foco 1 y Foco 2. Validación y guardado en Firebase, pausa de recarga mientras se edita y activación condicionada al dispositivo actualizado. Programa NodeMCU adaptado y compilado; instalación pospuesta por el usuario. 26 pruebas JavaScript y pruebas C++ del horario.

## 1.0.1 - 24 de octubre de 2026

Restaurada la recarga automatica cada diez segundos junto a los listeners en tiempo real. Espera los datos iniciales y escrituras pendientes; se pausa en pestanas ocultas y se comprueba al volver. Se revalida la pagina raiz. Cuatro pruebas nuevas (16 aprobadas en total).

## 1.0.0 — 23 de octubre de 2026

Primera base recuperada para appMichiWeb. Corregidos el tercer foco y las escrituras compartidas; añadidos listeners por nodo, estados de conexión/error, validación de lecturas y modos demo/solo lectura. Recursos activos locales, marcado accesible, configuración específica para Hosting y doce pruebas automatizadas. El contrato de datos se conserva. Publicación completada y lecturas reales verificadas en modo de solo lectura.
