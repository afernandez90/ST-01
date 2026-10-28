# Instalación física del cargador RDM — 27 de octubre de 2026

## Preparación y respaldo

Programador: segundo Mega con el ejemplo oficial ArduinoISP, STK500v1 a 19200 baudios. Objetivo: controlador Mega2560, firma `1E 98 01`. Conexiones y alimentación en el [manual del cargador](../arduino/arranque_rdm/LEEME.md).

Se observaron firmas erráticas y ceros con el cableado externo conectado; la identificación correcta se obtuvo tras aislar el controlador dejando solo los seis cables ISP/alimentación. Condensador utilizado en RESET del programador: 100 µF, positivo a RESET y negativo a GND.

Respaldo local, fuera del seguimiento Git, en `build/respaldos/mega-20261027/`: flash completa, EEPROM de 4096 bytes, firma, fusibles y bloqueos. `manifest.json` contiene los hashes SHA-256. Se conserva el ejemplo ArduinoISP y el script exacto utilizado para construir la imagen de instalación.

Lecturas previas:

| Parámetro | Valor |
|---|---|
| LFUSE | `0xFF` |
| HFUSE | `0xD8` |
| EFUSE | `0xFD` |
| LOCK | `0xFF` |
| Aplicación existente | 0.0.7 |
| Dirección DMX en EEPROM | 1 |
| Marca FTC, EEPROM 96 | `0xFF` |

Los fusibles ya seleccionan arranque en la región de 8 KiB; no se modifican. La imagen de instalación conserva todos los bytes de aplicación por debajo de `0x3E000` y sustituye la región superior con el nuevo cargador y su firma. El borrado ISP elimina EEPROM con EESAVE sin programar; se restaura desde el respaldo. Bloqueo previsto `0x2F`: impide SPM sobre el cargador, permite leer su firma desde la aplicación y permite actualizar la aplicación. Referencia: tablas 29-3, 29-13 y 30-1 de la [hoja de datos de Microchip](https://ww1.microchip.com/downloads/en/devicedoc/atmega640-1280-1281-2560-2561-datasheet-ds40002211a.pdf).

## Resultado

Instalación ISP completada: 39 520 bytes programados y verificados en flash; los 4096 bytes de EEPROM restaurados y verificados. Fusibles releídos sin cambios. Bloqueo escrito como `0x2F`, verificado y releído como `0xEF` (bits superiores no implementados a uno; seis bits efectivos `0x2F`). Pendientes la comprobación USB directa, una actualización FTC real, interrupción y recuperación, y preservación de dirección/etiqueta durante las actualizaciones. La aplicación 0.0.9 y su autocomprobación de arranque no se instalan en este paso: se conserva 0.0.7.

## Primera comprobación USB

El controlador arrancó normalmente por USB directo y anunció `[FTC] Cargador RDM compatible disponible.` La dirección persistente sigue siendo 001. El fallo LCD I2C -4 es coherente con haber desconectado el cableado externo. La identificación STK500v2 mediante avrdude devolvió correctamente `1E 98 01`.

La verificación USB de flash falló antes de leer datos: el cargador instalado intercambia los comandos `READ_FLASH_ISP` y `PROGRAM_EEPROM_ISP`. Según AVR068 y las constantes de avrdude, los valores correctos son 0x14 para leer flash y 0x15 para escribir EEPROM. No se realizaron escrituras USB. Se corrigió el código y se añadió una prueba nativa del mismo receptor USB con la trama real de avrdude, lectura/escritura flash y EEPROM, direcciones extendidas, límites y checksum.

El cargador corregido compila dentro de los 8 KiB y supera las pruebas, y se reinstaló por ISP en el paso descrito abajo. Pendiente verificar por USB esta segunda instalación. La primera imagen instalada y sus hashes permanecen en el respaldo; no sobrescribirlos al preparar la segunda instalación.

### Advertencia sobre el respaldo EEPROM por ArduinoISP

La comparación EEPROM por USB detectó que el respaldo inicial obtenido por STK500v1 no usa el mismo mapa de direcciones que la lectura USB directa. La lectura ISP muestra la marca de etiqueta en 16 y repite datos a +2048; la lectura USB directa con `CMD_LOAD_ADDRESS` en bytes y `READ_EEPROM_ISP` por bloques de ocho muestra la marca en 32. El ejemplo ArduinoISP multiplica `here` por dos para EEPROM y debe auditarse frente a avrdude 8 antes de volver a restaurar esa memoria. No reutilizar a ciegas `eeprom.hex` para una nueva instalación.

Se conserva la lectura física USB actual en `build/respaldos/mega-20261027/eeprom-usb-prueba.hex` y la traza en `usb-eeprom.log`. Antes de otro borrado ISP, cotejar este mapa mediante lecturas SPI universales con direcciones de byte explícitas y restaurarlo mediante esa misma ruta o un programador verificado. La configuración visible actual sigue en dirección 001 y la marca/etiqueta se encuentran en 32/33. El nuevo receptor USB tiene pruebas de EEPROM en direcciones de byte, además de la corrección de comandos.

## Segunda instalación: corrección USB y EEPROM preservada

La lectura SPI universal de los 4096 bytes de EEPROM coincide exactamente con la lectura USB anterior. Respaldo canónico: `eeprom-spi-universal.bin`. Se programó y verificó temporalmente HFUSE `0xD0` para activar EESAVE, se borró/programó la imagen corregida conservando la aplicación, y se restauró HFUSE `0xD8` y bloqueo efectivo `0x2F`.

Resultado: 39 488 bytes de flash programados y verificados. EEPROM releída por SPI universal después de instalar: **4096/4096 bytes idénticos al respaldo canónico**. Fusibles finales releídos: LFUSE `0xFF`, HFUSE `0xD8`, EFUSE `0xFD`; bloqueo leído `0xEF` (seis bits efectivos `0x2F`). No fue necesario restaurar EEPROM.

Se conservan `instalacion-correccion-usb.hex`, `arranque-corregido.hex`, ambas lecturas EEPROM y `manifest-correccion-usb.json` en el mismo respaldo local. Pruebas nativas de USB y del lector/restaurador EEPROM superadas. Pendiente reconectar el controlador directamente por USB y repetir verificación flash/EEPROM antes de probar escritura USB y actualización FTC real. Continúa instalada la aplicación 0.0.7.

## USB validado después de corregir

Con el controlador conectado directamente por USB, avrdude verificó 31 400 bytes de aplicación y los 4096 bytes del respaldo EEPROM canónico sin diferencias. Después se realizó una carga USB real de la misma aplicación 0.0.7 (`wiring`, STK500v2, 115200 baudios, sin borrado global): programación y verificación de los 31 400 bytes correctas.

EEPROM releída después de la carga: único cambio en el byte 96, de `0xFF` a `0xA5`, marca prevista de aplicación válida. Los otros 4095 bytes, incluidos dirección y etiqueta, son idénticos. Tras reinicio, el sketch vuelve a arrancar, conserva dirección 001 y anuncia cargador RDM compatible. El error LCD -4 es esperado mientras está desconectado. Diagnóstico local en `inicio-despues-carga-usb.txt`.

Estado actual: instalación ISP, identificación USB, lectura/verificación flash y EEPROM, escritura/verificación de aplicación por USB y reinicio normal comprobados físicamente. Pendiente actualización FTC real por RS-485, interrupción/recuperación y controladores externos. Sigue instalada 0.0.7; no se ha cargado 0.0.9.

## Primera prueba FTC física: fallo del reinicio watchdog

Con la interfaz RDM conectada, descubrimiento/silencio e información pasaron; el equipo reportó 0.0.7, dirección 1, etiqueta ST-01, 19 canales y FTC disponible (`rdm-antes-ftc.json`). Se empaquetó la aplicación original: 31 416 bytes incluidos los 16 de cabecera, CRC FTC `77FF`. La primera carga agotó el tiempo de espera durante la transición al cargador, sin llegar a enviar bloques de firmware; tampoco respondió el descubrimiento posterior.

El desensamblado del cargador instalado no contiene la rutina `.init3` para desactivar el watchdog: LTO había eliminado la función no referenciada. Esto impide atender RDM tras el reset watchdog utilizado por la aplicación, aunque un reset USB externo funciona. Se retiene explícitamente mediante `used` y `externally_visible`, con enlace C para identificarla de forma estable. El binario corregido incluye la secuencia WDR/desactivación WDTCSR antes de inicializar RAM.

La compilación comprueba ahora el desensamblado además de límites y firma y rechaza una imagen sin esa rutina temprana. Prueba negativa del verificador y pruebas del proyecto superadas. **Pendiente reinstalar por ISP la corrección watchdog**, preservando aplicación y EEPROM, y repetir la prueba FTC real. La actualización física no se considera validada.

## Instalación de la corrección watchdog por ISP

Se instaló el cargador con la rutina `.init3` retenida y comprobada en el desensamblado. Imagen `instalacion-watchdog.hex`: 39 504 bytes escritos y verificados, aplicación original 0.0.7 conservada. EESAVE se activó temporalmente y después se restauraron HFUSE `0xD8` y protección efectiva `0x2F`, ambos verificados por avrdude.

Respaldo EEPROM por SPI universal antes y después: **4096/4096 bytes idénticos**, también idénticos al estado después de la carga USB anterior. Se guardan `eeprom-antes-watchdog.bin`, `eeprom-despues-watchdog.bin`, imagen, ELF, desensamblado y `manifest-watchdog.json` en el respaldo local. Pendiente repetir arranque/verificación USB y transferencia FTC física con el nuevo cargador. No se ha instalado 0.0.9.

## FTC real después de corregir watchdog

El handover, transferencia completa, validación/commit y retorno a aplicación 0.0.7 se completaron correctamente. Pasaron comprobaciones RDM posteriores y pruebas de CRC/duplicados/offset/interrupción de enlace con posterior recarga completa. Resultados y límites en [pruebas FTC](PRUEBAS_FTC.md). Sigue pendiente el corte eléctrico real.

## Cierre de validación: recuperación eléctrica correcta

La prueba de corte real completó la validación del flujo principal: después de perder alimentación con carga parcial, el equipo permaneció en cargador y aceptó una nueva transferencia completa por RDM. Verificación/commit y reinicio a 0.0.7 correctos, configuración conservada y descubrimiento RDM normal restaurado. No se necesitó repetir ISP. Detalles y evidencias en [pruebas FTC](PRUEBAS_FTC.md).

Tras estas validaciones con 0.0.7, se cargó 0.0.9 por RDM con el nuevo lanzador y se confirmó versión y configuración por RDM. Ese es el firmware actual de la placa; véase [pruebas FTC](PRUEBAS_FTC.md).
