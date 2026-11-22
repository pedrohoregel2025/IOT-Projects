# appMichiWeb

Panel web de Pedro Hernández Oregel para controlar luces, consultar temperatura y humedad y ajustar el brillo del reloj Orizaba130 mediante Firebase Realtime Database y dispositivos NodeMCU ESP8266.

**Edición documental:** 22 de noviembre de 2026. **Web:** 1.3.6. **MichiCasa:** 2.3.1. **Reloj Orizaba130:** 1.0.2.

- [Proyecto en GitHub](https://github.com/pedrohoregel2025/IOT-Projects/tree/main/appMichiWeb)
- [Sitio existente](https://appmichiweb.web.app/)
- [Demostración del sitio](https://appmichiweb.web.app/?demo=1)

Esta entrega reúne el código web, tres perfiles de firmware, conexiones, configuración, pruebas e instrucciones de instalación. La copia descargable de README.MD y la de GitHub contienen la misma descripción. El robot, la ESP32-CAM y otros ejercicios del repositorio son proyectos separados.

## 1. Estado y alcance

| Elemento | Función | Situación documentada |
| --- | --- | --- |
| appMichiWeb 1.3.6 | Panel adaptable al celular, luces, sensores, horarios y brillo | Código recuperado de la versión funcional del proyecto |
| MichiCasa 2.3.1 | Focos 1/2, DHT11 y control físico local | Firmware integrado con la web; funcionamiento confirmado previamente por el propietario |
| Reloj Orizaba130 1.0.2 | Hora, calendario, temperatura, humedad, animaciones y brillo remoto | Sustituye al antiguo emisor de Orizaba130; requiere este firmware para confirmar brillo |
| Orizaba130 con relés | Focos 3/4 y DHT11 | Perfil alternativo anterior, conservado para reproducir ese montaje |
| Movimiento | Indicador leído de Firebase | Necesita su propio emisor; los tres firmwares incluidos no publican movimiento |
| Seguridad Firebase | Autenticación y reglas de acceso | Mejora pendiente de coordinar con los dispositivos |

**El reloj no controla los focos 3/4.** La web conserva sus controles y horario, pero requieren el dispositivo de relés correspondiente. No mantener el antiguo emisor Orizaba130 y el reloj publicando simultáneamente en la misma ruta de sensores.

La preparación de esta entrega comprueba software, no sustituye una prueba física actual. Las confirmaciones históricas del propietario y las verificaciones nuevas se distinguen en [VALIDACION.md](VALIDACION.md).

## 2. Arquitectura

```text
Celular o computadora
        │ HTTPS
        ▼
appMichiWeb / Firebase Hosting
        │ Firebase SDK: lecturas y órdenes
        ▼
Firebase Realtime Database
        ├── MichiCasa: focos 1/2 + DHT11
        ├── Reloj Orizaba130: DHT11 + RTC + matriz
        └── Alternativa: Orizaba130 con relés para focos 3/4
```

Los dispositivos se conectan por WiFi de 2.4 GHz. El navegador no se conecta directamente a sus pines. Los horarios los ejecuta el controlador de relés después de recibir configuración y hora válida; no hace falta mantener abierta la página. No se necesita un servidor Node.js para ejecutar el sitio.

## 3. Funciones de la web

1. **Luces exteriores:** Foco 1, Calle; Foco 2, Garage; un horario diario compartido.
2. **Sensores:** temperatura y humedad de Orizaba130 y MichiCasa. MichiCasa conserva la ruta histórica `Farm/RigRojo`.
3. **Otros dispositivos:** Focos 3/4, movimiento, su horario independiente y, al final de la sección, el control de brillo del reloj.

La interfaz muestra conexión, escrituras pendientes, errores y confirmación del dispositivo. Los datos se actualizan mediante suscripciones Firebase. La recarga completa está configurada a 60 segundos y se pospone durante ediciones o escrituras para no perder cambios. Esto no equivale a un historial: se guarda la lectura más reciente.

### Modos

| Modo | Acceso | Efecto |
| --- | --- | --- |
| Demostración local | `http://127.0.0.1:8765` o localhost | Datos simulados, sin escribir en Firebase ni accionar equipos |
| Demostración publicada | Añadir `?demo=1` | Simulación independiente del hardware |
| Solo lectura | Añadir `?readonly=1` a un sitio configurado | Lee datos reales; esta interfaz no escribe controles, horarios ni reloj |
| Operación | Sitio configurado sin esos parámetros | Lee y escribe según los permisos de Firebase |

**Solo lectura es una opción de interfaz, no una medida de autorización.** Tampoco la configuración web de Firebase constituye una contraseña de acceso a la base.

## 4. Requerimientos de software

| Componente | Versión de referencia / uso |
| --- | --- |
| Navegador | JavaScript habilitado y soporte de SVG, Promise y URLSearchParams |
| Python 3 | Servidor HTTP local para la demostración |
| Node.js | 18 o posterior para `node --test`; no se requieren paquetes npm para las pruebas web |
| Firebase web SDK | 7.15.5, copia local en `public/vendor` |
| Firebase CLI | Solo para configurar y publicar Hosting; usar una versión compatible con el Node instalado |
| Arduino IDE o Arduino CLI | Compilación/carga de cada NodeMCU |
| Núcleo ESP8266 | 3.1.2; placa **NodeMCU 1.0 (ESP-12E Module)** |
| Firebase ESP8266 Client | 4.4.17, API existente del firmware |
| MD_MAX72XX | 3.5.1, únicamente para el reloj |
| RTClib | 2.1.4, únicamente para el reloj |
| Adafruit BusIO | 1.17.4, dependencia del RTC |
| DHTD0 | Incluido con su licencia; adaptación para GPIO16 basada en Adafruit DHT |

Las versiones anteriores son las usadas por este proyecto, no una afirmación de que sean las más recientes. Actualizarlas requiere volver a probar compatibilidad. No reemplazar DHTD0 por DHT.h sin adaptar y comprobar GPIO16.

## 5. Materiales

### MichiCasa

- Un NodeMCU Amica o compatible, ESP8266 ESP-12E/ESP-12F.
- Placa Laika LK001 con dos relés, o circuito eléctricamente equivalente con entradas de 3.3 V y activación HIGH.
- Un DHT11 compatible con alimentación/señal de 3.3 V.
- Dos pulsadores y dos indicadores LED, incorporados a la placa o cableados con sus resistencias.
- Fuente regulada adecuada, cable USB de datos, cables y montaje aislado.

### Reloj Orizaba130

- Otro NodeMCU ESP8266.
- Una matriz MAX7219 tipo FC16, cuatro módulos 8×8 en cadena: 32×8 LED.
- Un módulo RTC DS3231 compatible con 3.3 V y pila de respaldo apropiada para su circuito.
- Un DHT11, un pulsador y cables.
- Alimentación regulada de 5 V con capacidad para la matriz y el NodeMCU.
- Adaptación de nivel para DIN, CLK y CS: por ejemplo tres canales de un 74AHCT125 alimentado a 5 V, con desacoplo de 100 nF y OE a GND.

### Alternativa Orizaba130 con relés

Un NodeMCU, dos relés compatibles, un DHT11 y su alimentación. Este montaje ocupa el lugar del reloj como emisor de `Farm/Orizaba130`; no es un programa para cargar junto con el reloj en la misma placa.

## 6. Conexiones MichiCasa

Los nombres D0, D1, etc. son las etiquetas de la NodeMCU; no equivalen al número GPIO.

| Función | NodeMCU | GPIO | Comportamiento del código |
| --- | --- | --- | --- |
| Foco 1 / relé 1 | D2 | 4 | HIGH activa, LOW desactiva |
| Foco 2 / relé 2 | D1 | 5 | HIGH activa, LOW desactiva |
| DHT11 DATA | D0 | 16 | Usar DHTD0 incluido |
| Pulsador 1 | D5 | 14 | Entre el pin y GND; pulsado LOW |
| Pulsador 2 | D7 | 13 | Entre el pin y GND; pulsado LOW |
| LED 1 | D6 | 12 | Indicador activo LOW con Internet |
| LED 2 | D8 | 15 | Indicador activo LOW con Internet |
| Sensor VCC/GND | 3V3 / GND | — | Verificar las etiquetas del módulo real |

La asignación corresponde al firmware, no garantiza el cableado interno de cualquier revisión de LK001. D8/GPIO15 debe permanecer LOW al arrancar: no añadir un circuito que lo fuerce HIGH. Si se usan LED externos, incluir resistencia limitadora y verificar que no alteren el arranque. Las entradas de los relés necesitan su etapa de manejo; nunca conectar una bobina directamente a un GPIO.

Los pulsadores funcionan sin Internet. Sin acceso a Internet los dos LED parpadean de forma sincronizada; esto no cambia por sí mismo el estado de los relés.

## 7. Conexiones del reloj

| Dispositivo / terminal | NodeMCU o alimentación | GPIO / detalle |
| --- | --- | --- |
| Matriz IN: DIN | D7 mediante adaptador de nivel | 13 |
| Matriz IN: CLK | D5 mediante adaptador de nivel | 14 |
| Matriz IN: CS / LOAD | D6 mediante adaptador de nivel | 12 |
| Matriz VCC | 5 V regulados | Nunca 9 V |
| Matriz GND | GND común | Compartido con NodeMCU y sensores |
| DS3231 SDA | D2 | 4 |
| DS3231 SCL | D1 | 5 |
| DS3231 VCC / GND | 3V3 / GND | Pull-ups I2C a 3.3 V |
| DHT11 DATA | D0 | 16 |
| DHT11 VCC / GND | 3V3 / GND | Pull-up DATA de 4.7–10 kΩ si el módulo no lo incorpora |
| Pulsador | D3 y GND | 0, INPUT_PULLUP; pulsado LOW |

El ESP8266 usa señales de 3.3 V. El MAX7219 alimentado a 5 V necesita adaptación para garantizar el nivel HIGH; alimentar la matriz a 5 V no vuelve tolerantes a 5 V los pines del ESP8266. Conectar la matriz por el lado IN.

D3/GPIO0 participa en el arranque: soltar el pulsador y FLASH al encender, reiniciar o programar. En pulsadores de cuatro patas, comprobar qué contactos se unen al presionar.

No instalar una CR2032 primaria en un módulo RTC que intente recargarla. La pila del DS3231 mantiene el reloj, no alimenta el NodeMCU ni la matriz. El orden físico S/+/- del DHT puede cambiar entre módulos: seguir sus etiquetas.

### Presentación y configuración horaria

- Cada pulsación cambia **Hora → Temperatura → Humedad → Calendario → Hora**.
- Minuto 00: calendario. Minutos 10, 20, 30, 40 y 50: temperatura y humedad.
- Cada cinco minutos: una animación; hay cinco efectos y se evita repetir el mismo consecutivamente.
- Coincidencias: calendario, datos y animación en ese orden.
- NTP usa `pool.ntp.org`, `time.google.com` y `time.cloudflare.com`.
- El RTC almacena UTC y la pantalla aplica UTC−6. No ajustar manualmente el RTC a hora local con este firmware.
- Con RTC válido, conserva hora sin Internet. Sin hora válida muestra `--:--`; no presenta la fecha de compilación como hora real.
- Conectar a Internet al menos una vez para inicializar el RTC y después comprobar su respaldo con pila.

El reloj es de ejecución cooperativa: una petición de red puede retrasar una animación o el procesamiento de un botón.

## 8. Conexiones de la alternativa Orizaba130 con relés

| Función | NodeMCU | Ruta |
| --- | --- | --- |
| Relé 1, activo HIGH | D2 / GPIO4 | `casa1/foco3` |
| Relé 2, activo HIGH | D1 / GPIO5 | `casa1/foco4` |
| DHT11 DATA | D0 / GPIO16 | `Farm/Orizaba130` |
| DHT VCC / GND | 3V3 / GND | Según módulo compatible |

Comparte los criterios de alimentación y aislamiento del montaje MichiCasa. No confundir este programa con `RelojOrizabaRTC`.

## 9. Armado y puesta en marcha

1. Elegir el perfil de dispositivo y revisar su tabla de pines antes de conectar.
2. Montar con todas las fuentes y el USB desconectados. Comprobar polaridad, GND común y ausencia de cortocircuitos.
3. Para el reloj, conectar RTC, sensor, pulsador y matriz con su adaptación de nivel. Para relés, comenzar con cargas de baja tensión y comprobar HIGH/LOW.
4. Preparar `secrets.h`, compilar y cargar únicamente en la placa correspondiente. Una carga sustituye su programa anterior.
5. Abrir el monitor serie a **115200 baudios**. Reiniciar sin mantener FLASH ni el pulsador D3.
6. Configurar una red WiFi de 2.4 GHz. Comprobar sincronización, lectura de sensor y publicación de datos.
7. Probar primero la web en demostración, después lecturas reales y finalmente una orden individual.
8. Verificar pérdida de Internet, reinicio y recuperación antes del montaje definitivo.

Las pruebas de relés con tensión de red requieren una instalación aislada y protegida realizada por personal competente. La corriente impresa en el relé no determina por sí sola la capacidad segura de toda la placa, pistas, bornes o caja.

## 10. Ejecutar la web localmente

Desde la carpeta `appMichiWeb`:

```sh
python3 -m http.server 8765 --bind 127.0.0.1 --directory public
```

Abrir `http://127.0.0.1:8765`. En localhost se activa demostración automáticamente, incluso si se agrega `?live=1`; esto evita accionar equipos accidentalmente durante la revisión local.

Alternativa con Node/npm instalados:

```sh
npm run serve
npm test
```

No es necesario ejecutar `npm install` para la web. No abrir directamente `index.html` como sustituto de un servidor HTTP.

## 11. Configuración Firebase y publicación

La entrega pública contiene **valores de ejemplo**, no las credenciales del sistema existente. La demostración funciona sin completarlos. La operación con hardware requiere configuración propia y autorización correcta en la base.

1. En tu proyecto Firebase, registrar o seleccionar una aplicación web y habilitar Realtime Database y Hosting según tus necesidades.
2. Completar `public/js/app-config.js` a partir de `app-config.example.js` con la configuración web de ese proyecto. Estos identificadores se entregan al navegador; no insertar secretos administrativos ni claves de cuenta de servicio.
3. Diseñar y comprobar autenticación/reglas antes de exponer datos reales. Esta versión de la interfaz no implementa inicio de sesión Firebase Auth; un proyecto nuevo con reglas cerradas requiere completar esa integración para operar. No usar reglas globales abiertas para resolver un error de permisos.
4. Para Hosting, con Firebase CLI instalado:

```sh
firebase login
firebase use --add
firebase target:apply hosting michi TU_ID_DE_SITIO
firebase deploy --only hosting:michi
```

Ejecutar dentro de `appMichiWeb` y comprobar el proyecto seleccionado antes de publicar. `firebase.json` contiene solo Hosting: este comando no despliega reglas de la base. `.firebaserc.example` muestra el formato; `.firebaserc` local se excluye de Git.

El sitio existente usa Firebase Hosting y Realtime Database. Los horarios se ejecutan en los ESP8266; el diseño no necesita Cloud Scheduler ni Cloud Functions. El coste depende del plan, cuotas y uso de Firebase; la entrega no promete consumo ilimitado ni activa un plan de pago.

### Situación de seguridad del sistema existente

La revisión del **17 de noviembre de 2026** detectó permisos globales de lectura y escritura públicos. El propietario decidió posponer los cambios hasta disponer de los dispositivos. Esta publicación documental no cambia esas reglas, no activa autenticación ni modifica el sitio alojado.

Los firmwares actuales conservan la integración heredada mediante `FIREBASE_AUTH`; su migración debe coordinarse con la autenticación de la web. No publicar ese valor, `secrets.h`, respaldos de flash ni binarios compilados con claves. Una API key web no reemplaza las reglas de acceso. El estado de seguridad debe volver a verificarse antes de producción.

## 12. Preparar y cargar los firmwares

Los sketches se encuentran en:

```text
firmware/NodeMCUProyectoLuz/NodeMCUProyectoLuz.ino
firmware/RelojOrizabaRTC/RelojOrizabaRTC.ino
firmware/Orizaba130/Orizaba130.ino
```

En **cada** carpeta utilizada, copiar `secrets.example.h` como `secrets.h` y completar:

| Constante | Contenido privado requerido |
| --- | --- |
| FIREBASE_HOST | Host RTDB sin `https://`, tomado de la consola; puede variar según región |
| FIREBASE_AUTH | Credencial compatible con la integración heredada; no es la API key web |
| WIFI_SSID | Nombre de tu red de 2.4 GHz |
| WIFI_PASSWORD | Clave de esa red |
| CONFIG_AP_PASSWORD | Clave propia de 8–63 caracteres para el portal local de MichiCasa o del reloj |

La clave del portal se obtiene ahora de `secrets.h`; las contraseñas fijas del montaje privado no se distribuyen. Se conserva el comportamiento del firmware y se añade esa separación de configuración para la entrega pública.

### Arduino IDE

1. Añadir a los índices de placas `https://arduino.esp8266.com/stable/package_esp8266com_index.json`.
2. Instalar ESP8266 **3.1.2** y las bibliotecas de la sección 4.
3. Seleccionar **NodeMCU 1.0 (ESP-12E Module)** y el puerto USB correcto.
4. Abrir el `.ino` de la carpeta elegida, verificar y cargar. No conectar dos placas indistinguibles durante la selección del puerto.

### Arduino CLI

Después de instalar el núcleo y bibliotecas correspondientes, desde `appMichiWeb`:

```sh
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 firmware/NodeMCUProyectoLuz
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 firmware/RelojOrizabaRTC
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 firmware/Orizaba130
```

Ejemplo de carga, sustituyendo el puerto por el mostrado en `arduino-cli board list`:

```sh
arduino-cli upload --port TU_PUERTO_USB --fqbn esp8266:esp8266:nodemcuv2 firmware/RelojOrizabaRTC
```

No se incluyen los instaladores antiguos con rutas específicas de un Mac ni respaldos privados. Los comandos de ejemplo no identifican automáticamente qué montaje está conectado.

### Portal WiFi local

| Dispositivo | SSID de configuración | Dirección | Contraseña |
| --- | --- | --- | --- |
| MichiCasa | `MichiCasa-Config` | `http://192.168.4.1` | La elegida en CONFIG_AP_PASSWORD |
| Reloj | `OrizabaReloj-Config` | `http://192.168.4.1` | La elegida en CONFIG_AP_PASSWORD |

El AP permanece disponible incluso con Internet. Conectar el celular, aceptar permanecer en una red sin Internet, abrir la dirección manualmente y guardar la red doméstica de 2.4 GHz. Si la asociación falla, se conserva el perfil previo. El perfil guardado tiene prioridad sobre el compilado. La EEPROM no cifra la contraseña. El perfil alternativo Orizaba130 con relés usa la red definida en `secrets.h`, sin este portal.

## 13. Contrato de Realtime Database

Conservar mayúsculas y nombres: forman parte de la compatibilidad entre firmware y web.

| Ruta | Tipo / propósito | Responsable |
| --- | --- | --- |
| `casa1/foco1`, `casa1/foco2` | Boolean: estado/orden | Web y MichiCasa |
| `casa1/foco3`, `casa1/foco4` | Boolean: estado/orden | Web y controlador alternativo de relés |
| `casa1/movimiento` | Boolean o valor compatible 0/1 | Emisor externo; web lee |
| `casa1/horarioLuces` | Horario de focos 1/2 | Web configura, MichiCasa ejecuta |
| `casa1/horarioLucesEstado` | Disponibilidad, reloj y última comunicación | MichiCasa |
| `casa1/horarioLuces34` | Horario de focos 3/4 | Web y controlador alternativo |
| `casa1/horarioLuces34Estado` | Disponibilidad del controlador 3/4 | Controlador alternativo |
| `Farm/RigRojo/Temperatura`, `Humedad` | Números; etiqueta visible MichiCasa | MichiCasa |
| `Farm/Orizaba130/Temperatura`, `Humedad` | Números | Reloj o emisor alternativo, nunca ambos |
| `Farm/<equipo>/estado` | `lectura_valida` / `error_sensor` | Firmware |
| `Farm/<equipo>/actualizadoEn` | Timestamp del servidor en ms de publicación válida | Firmware |
| `Farm/<equipo>/origen` | Identificador del emisor, cuando se publica | Firmware |
| `dispositivos/Orizaba130/matriz/brillo` | Entero 0–15 solicitado | Web |
| `dispositivos/Orizaba130/matriz/estado` | Brillo aplicado, firmware y actualizadoEn | Reloj 1.0.2 |
| `timer/day`, `month`, `year`, `hours`, `minutets` | Sincronización heredada desde navegador | Web en operación |

`minutets` es la escritura histórica del nombre; no corregirla aisladamente. El reloj nuevo usa NTP/RTC independientemente del nodo `timer`.

Ejemplo de horario, no de reglas de acceso:

```json
{
  "schemaVersion": 1,
  "configured": true,
  "enabled": true,
  "timeZone": "America/Mexico_City",
  "on": { "hour": 19, "minute": 0 },
  "off": { "hour": 7, "minute": 0 },
  "updatedAt": 0
}
```

En uso, la app asigna `updatedAt` mediante timestamp del servidor. Las horas de encendido y apagado deben diferir. Se admiten períodos que cruzan medianoche. Al activar/cambiar el horario se aplica el período actual; los cambios manuales se conservan hasta la siguiente transición programada. El horario descargado queda en RAM: tras un reinicio el controlador necesita recuperar red, hora y configuración. No confundir continuidad sin Internet con persistencia después de cortar energía.

La activación de focos 3/4 exige disponibilidad reciente del controlador. El primer grupo permite guardar la intención pendiente. Los sensores se muestrean/publican aproximadamente cada diez segundos; ante fallo se invalida la lectura en lugar de mostrar un cero ficticio.

### Brillo del reloj

0 es el mínimo encendido, no apagado; 15 es el máximo. El reloj consulta aproximadamente cada cinco segundos, confirma cada treinta segundos o después de un cambio y guarda en EEPROM solo los cambios válidos. Tras reiniciar puede recuperar el último brillo sin Internet. La web confirma aplicación únicamente cuando el reloj informa el mismo valor con una marca reciente, dentro de 90 segundos. Guardar en Firebase no demuestra por sí solo que el reloj lo aplicó.

## 14. Organización del código

```text
appMichiWeb/
├── README.MD                 Descripción concentrada del proyecto
├── VALIDACION.md             Resultado de verificaciones de esta entrega
├── CHANGELOG.md              Historial de la web
├── DEPENDENCIAS.json         Procedencia y hashes de archivos web de terceros
├── THIRD_PARTY_NOTICES.md    Avisos y referencias de licencias
├── package.json              Comandos de prueba, servidor y Hosting
├── firebase.json             Configuración Hosting, sin reglas RTDB
├── .firebaserc.example       Ejemplo de proyecto/sitio
├── public/
│   ├── index.html
│   ├── css/app.css
│   ├── img/
│   ├── js/                   Vista, datos, horarios, brillo y configuración
│   └── vendor/               SDK Firebase local
├── tests/                    Pruebas JavaScript
├── scripts/test-firmware.py  Pruebas C++ sin conectar hardware
└── firmware/
    ├── NodeMCUProyectoLuz/   MichiCasa
    ├── RelojOrizabaRTC/      Reloj actual
    ├── Orizaba130/           Alternativa anterior con relés
    └── tests/                Pruebas C++ de lógica
```

`app-core.js` contiene el contrato de datos y coordinación de controles; `main.js` conecta la vista; `schedule.js` gestiona horarios y `brightness.js` el brillo. El sitio utiliza JavaScript y SVG nativos. Se conservan recursos auxiliares históricos que no son dependencias activas de la página.

## 15. Pruebas y diagnóstico

```sh
npm test
python3 scripts/test-firmware.py
```

Las pruebas C++ necesitan `c++` con soporte C++11. Las compilaciones completas de firmware necesitan además el núcleo y bibliotecas de Arduino. Ver [VALIDACION.md](VALIDACION.md) para los resultados de la preparación.

| Síntoma | Revisar |
| --- | --- |
| La demostración cambia datos pero no el equipo | Es intencional; comprobar modo y configuración real |
| `PERMISSION_DENIED` | Proyecto, autenticación y reglas; no abrir permisos globales como arreglo |
| Controles esperando o deshabilitados | Conexión RTDB, rutas y valores booleanos; revisar error visible |
| Horario 3/4 no disponible | El reloj no es el controlador de esos relés; comprobar heartbeat del dispositivo correcto |
| Sensor sin lectura válida | VCC/GND/DATA, etiquetas físicas, pull-up a 3.3 V y DHTD0 |
| Brillo guardado sin confirmar | Firmware 1.0.2, conectividad, rutas de matriz y estado reciente |
| Reloj `--:--` | NTP inicial, RTC válido, I2C, pila y señal de pérdida de energía |
| NodeMCU no arranca | D3/FLASH pulsado, estado de pines de arranque o alimentación |
| Matriz inestable o reinicios | Fuente bajo carga, GND común, adaptación de nivel, orientación IN y brillo |
| Cambio WiFi no persiste | Red 2.4 GHz, clave correcta y asociación exitosa antes de guardar |

Prueba física de aceptación: verificar ambos relés individualmente con carga de baja tensión; desconectar Internet y probar pulsadores; comprobar horario con una transición cercana; revisar sensor y recuperación; probar brillo 0/3/8/15; reiniciar sin Internet y comprobar RTC y brillo persistente. No afirmar que una prueba de compilación verifica la alimentación o el cableado.

## 16. Historial y próximos pasos

La web llegó a 1.3.6 incorporando horarios independientes, lecturas MichiCasa/Orizaba130, confirmación de brillo y su ubicación al final de la tercera sección. MichiCasa 2.3.1 incorpora control físico sin Internet y configuración WiFi persistente. El reloj 1.0.2 añade brillo remoto con respaldo local. El detalle de la evolución web está en [CHANGELOG.md](CHANGELOG.md).

Pendientes: coordinar autenticación y cierre de permisos Firebase con los equipos; decidir si habrá un controlador activo para focos 3/4; revisar dependencias antiguas antes de una actualización mayor; mantener resultados de pruebas físicas de cada instalación. No se publica un historial de sensores ni una integración nueva de movimiento con esta entrega.

## 17. Autoría, licencias y referencias

Proyecto de **Pedro Hernández Oregel**, con evolución de programas originales de su repositorio IOT-Projects. No se ha asignado una licencia nueva al código propio: publicar el código no otorga automáticamente una licencia de reutilización. Se preservan las licencias incluidas y avisos de componentes de terceros; consultar [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

- [Documentación de ESP8266 Arduino Core](https://arduino-esp8266.readthedocs.io/)
- [Firebase Realtime Database](https://firebase.google.com/docs/database/web/start)
- [Reglas de seguridad RTDB](https://firebase.google.com/docs/database/security)
- [Firebase Hosting](https://firebase.google.com/docs/hosting)
- [Firebase ESP8266 Client](https://github.com/mobizt/Firebase-ESP8266)
- [MD_MAX72XX](https://github.com/MajicDesigns/MD_MAX72XX)
- [Adafruit RTClib](https://github.com/adafruit/RTClib)
- [Ficha DS3231](https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf)
- [Ficha MAX7219/MAX7221](https://www.analog.com/media/en/technical-documentation/data-sheets/max7219-max7221.pdf)
