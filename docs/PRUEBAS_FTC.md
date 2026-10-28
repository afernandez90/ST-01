# Pruebas físicas de actualización FTC

Fecha: 27 de octubre de 2026. Controlador Mega2560 con cargador propio de 8 KiB, adaptador RS-485 automático y herramienta del proyecto con interfaz DMX/RDM. Se utilizó la aplicación original **0.0.7**, conservando dirección 001, etiqueta ST-01 y 19 canales. Estas pruebas de recuperación se realizaron antes de actualizar a 0.0.9.

| Comprobación | Resultado |
|---|---|
| Descubrimiento/silencio e información RDM | Correctos, FTC disponible |
| Cambio de aplicación al cargador por watchdog | Correcto tras corregir retención `.init3` con LTO |
| Transferencia completa | 31 416 bytes de archivo ST01 (31 400 de aplicación) |
| CRC de archivo y CRC32 de imagen recibida/grabada | Validación aceptada por el cargador antes de commit |
| Commit y retorno a aplicación | Correctos, versión 0.0.7 reportada por RDM |
| Configuración | Dirección 1 y etiqueta ST-01 conservadas |
| IDENTIFY, dirección, etiqueta y límites después de actualizar | Correctos, configuración restaurada |
| CRC incorrecto de un bloque | Rechazado con estado 11 |
| Último bloque duplicado | Aceptado sin avanzar dos veces el desplazamiento |
| Bloque fuera de secuencia | Rechazado con estado 0x14 y desplazamiento esperado 384 |
| Interrupción/reconexión del enlace de control | El cargador conserva sesión y desplazamiento; recarga completa y reinicio correctos |
| Corte eléctrico durante la carga y recuperación posterior | Correctos: permanece en cargador, acepta recarga completa y vuelve a 0.0.7 |

El paquete utilizado tiene CRC FTC `77FF`. Informes locales en `build/respaldos/mega-20261027/`: `rdm-antes-ftc-watchdog.json`, `rdm-despues-ftc-watchdog.json`, `recuperacion-ftc.json` y su registro. La prueba de enlace cierra y reabre la conexión del ordenador: no equivale a un corte de alimentación del Mega.

## Repetición

La siguiente prueba **escribe parcialmente la aplicación y después la carga completa**; utilizar un paquete compatible y disponer de recuperación USB/ISP:

```sh
python3 herramientas/probar_recuperacion_ftc.py ARCHIVO.st01 --puerto PUERTO_RDM --informe INFORME.json
```

Para preparar un corte físico, añadir `--preparar-corte`: deja 384 bytes transferidos, aplicación pendiente y equipo en el cargador. Apagar completamente Mega y módulo RS-485, volver a alimentar y ejecutar una carga completa mediante `firmware_rdm.py cargar`. No se reanuda automáticamente el archivo parcial después del corte; se reinicia desde cero.

Los resultados validan este montaje y la herramienta del proyecto. Continúan pendientes la medición eléctrica de temporización A/B y las pruebas con controladores externos compatibles con FTC 1.00. No se deduce certificación ni interoperabilidad universal.

## Corte eléctrico real

Se transfirieron 384 bytes del paquete y se dejó la aplicación pendiente. El usuario cortó y restauró la alimentación. Tras el corte, el equipo respondió por RDM como `ST01 cargador 1.0`, con dirección 1 y etiqueta ST-01: no arrancó una aplicación marcada incompleta. Se inició una sesión nueva desde cero, se transfirieron los 31 416 bytes, se validó y confirmó la imagen, y el equipo volvió a reportar `0.0.7 RDM prototipo`.

Después de recuperar, descubrimiento/silencio, FTC disponible, firmware, dirección, etiqueta y personalidad coinciden con los valores previos. Informes: `preparacion-corte-ftc.json`, `estado-tras-corte-ftc.json`, `recarga-tras-corte-ftc.log` y `rdm-despues-corte-ftc.json` en el respaldo local. No se necesitó USB ni ISP para recuperar después del corte.

## Actualización posterior a 0.0.9 por RDM

Después de validar recuperación, se utilizó `herramientas/cargar_rdm.sh` para compilar, empaquetar y cargar la última revisión 0.0.9 por RDM, con 32 398 bytes de archivo ST01. Validación, commit y reinicio correctos. La consulta posterior confirma modelo/personalidad/versión 0.0.9, 19 canales, dirección 1, etiqueta ST-01 y FTC disponible. Se conservaron `carga-v0.0.9-rdm.log` y `rdm-v0.0.9.json` en el respaldo local.
