# Validación de la entrega

Fecha: **22 de noviembre de 2026**. Se verificó la copia preparada para el repositorio, con configuración y credenciales de ejemplo. No se accedió a sensores ni se enviaron órdenes a los dispositivos.

## Resultados

| Verificación | Resultado |
| --- | --- |
| Pruebas JavaScript, `node --test tests/*.test.cjs` | 34 aprobadas, 0 fallidas |
| Pruebas C++, `python3 scripts/test-firmware.py` | 6 programas aprobados |
| MichiCasa / NodeMCUProyectoLuz | Compilación correcta para ESP8266 3.1.2, nodemcuv2 |
| RelojOrizabaRTC | Compilación correcta para ESP8266 3.1.2, nodemcuv2 |
| Orizaba130 con relés | Compilación correcta para ESP8266 3.1.2, nodemcuv2 |
| Referencias de recursos en index.html | Todos los archivos locales referenciados existen |
| Revisión de secretos en archivos a publicar | Sin secrets.h, binarios, claves privadas, tokens reconocibles ni contraseñas originales de los AP |

Bibliotecas usadas en compilación: Firebase ESP8266 Client 4.4.17; para el reloj, MD_MAX72XX 3.5.1, RTClib 2.1.4 y Adafruit BusIO 1.17.4. DHTD0 está incluido. Las copias de compilación usaron únicamente `secrets.example.h` con valores de ejemplo; los binarios de comprobación no se distribuyen.

## Recursos de compilación

| Firmware | RAM estática | IRAM, incluida caché de instrucciones | Código en flash |
| --- | --- | --- | --- |
| MichiCasa | 38 616 / 80 192 bytes | 60 867 / 65 536 bytes | 542 412 / 1 048 576 bytes |
| Reloj | 39 784 / 80 192 bytes | 61 531 / 65 536 bytes | 552 060 / 1 048 576 bytes |
| Orizaba130 relés | 38 436 / 80 192 bytes | 60 855 / 65 536 bytes | 512 288 / 1 048 576 bytes |

La ocupación de IRAM es alta; ampliaciones o cambios de bibliotecas deben volver a compilarse y probarse. Estas cifras no miden el margen real de memoria dinámica durante conexiones TLS.

## Alcance de las pruebas

JavaScript: aislamiento de escrituras por foco, conexión/reconexión, errores, controles pendientes, lecturas inválidas, modos demo/solo lectura, reloj heredado, recarga, horarios independientes, borradores y brillo confirmado.

C++: horarios diurnos/nocturnos y medianoche, prioridad manual, rebotes, sincronización de cambios locales, validación de sensores, perfiles WiFi, persistencia de brillo y política del reloj/animaciones durante 24 horas simuladas.

Las pruebas puras no comprueban WiFi real, permisos RTDB, corriente, adaptación de nivel, relés, sensor, RTC, pila ni cableado. La confirmación previa del propietario corresponde a la operación histórica del proyecto. No se cargó firmware nuevo en las placas.

No se realizó una revisión visual nueva del navegador en esta preparación: el entorno impidió abrir el servidor HTTP local. Sí se comprobaron las pruebas de lógica y las referencias estáticas de los recursos. La demostración puede abrirse siguiendo README.MD en un equipo que permita el servidor local.

## Límites de esta publicación

- La configuración pública se entrega con marcadores; para uso real se necesitan valores propios.
- El endurecimiento de Firebase sigue pendiente de coordinación con los dispositivos. Esta entrega no lo da por resuelto.
- El reloj no ejecuta focos 3/4; ese uso necesita el perfil alternativo de relés.
- No se desplegó Hosting, no se cambiaron reglas de la base y no se habilitó un plan de pago.
- La publicación en GitHub no modifica otros programas ya existentes en el repositorio.
