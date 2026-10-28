# Cargador RDM del Arduino Mega 2560

Cargador experimental de 8 KiB para ATmega2560 a 16 MHz. La misma imagen sirve independientemente de la marca del controlador RDM: el protocolo reside en el Arduino. No actualiza el firmware de consolas, interfaces ni otros equipos externos.

**Estado: instalado y verificado por ISP, carga/verificación USB y actualización FTC real por RDM validadas con la aplicación 0.0.7.** CRC de bloque incorrecto, duplicados, desplazamientos fuera de secuencia e interrupción/reconexión del enlace comprobados físicamente. Corte eléctrico durante carga y recuperación completa por RDM también validados. Pendientes medición de tiempos y controladores externos. Véanse [pruebas FTC](../../docs/PRUEBAS_FTC.md) y [registro de instalación y correcciones](../../docs/INSTALACION_ARRANQUE_RDM.md).

## Compilación

```sh
sh arduino/arranque_rdm/compilar.sh
```

El guion utiliza AVR GCC instalado con Arduino AVR Boards y produce `build/arranque_rdm/arranque.hex`. No programa la placa ni modifica fusibles. Puede especificarse `ST01_AVR_TOOLS` para otra instalación. `ST01_RS485_MANUAL=1` compila la variante DE+/RE en pin 27; usar la misma definición `-DST01_RS485_MANUAL` al compilar el sketch. La variante predeterminada corresponde al adaptador automático.

El verificador comprueba también que la desactivación watchdog temprana existe en el desensamblado antes de inicializar RAM (retención explícita con LTO), y que todas las direcciones pertenecen a `0x3E000–0x3FFFF`, y que la firma `ST01RDM1` está en `0x3FFF0`. El tamaño máximo de aplicación permanece en 253 952 bytes. El paquete FTC añade una cabecera de 16 bytes que no se graba como código.

## Instalación inicial por ISP

Requiere un programador ISP o un segundo Arduino con ArduinoISP. No es una carga normal de sketch por USB. Antes de instalar:

1. Leer y conservar la firma del chip, flash completa, EEPROM, fusibles y bloqueos actuales mediante el programador elegido.
2. Preparar una imagen completa que conserve la aplicación y reemplace únicamente los 8 KiB del cargador; conservar la EEPROM. Un borrado ISP del chip puede eliminar la aplicación y la EEPROM según los fusibles.
3. Configurar arranque en el bloque de 8 KiB. Proteger el cargador frente a SPM, permitiendo su lectura desde la aplicación para detectar la firma. Verificar los valores contra la hoja de datos del ATmega2560 y las lecturas reales antes de programarlos; no copiar fusibles de otra placa.
4. Programar y verificar flash/EEPROM/fusibles, comprobar carga y verificación USB, reinicio normal y recuperación RDM tras una interrupción de transferencia.
5. Validar también el retorno RS-485 automático. La recepción DMX ya comprobada no confirma que el DollaTek transmita y libere correctamente el bus.

La primera instalación se ha realizado conservando los fusibles leídos. Los resultados, bloqueos y respaldos se registran en [instalación física](../../docs/INSTALACION_ARRANQUE_RDM.md). No reutilizar valores de fusibles sin leer y comprobar la placa objetivo.

## Transferencia y recuperación

Se implementan FTC 1.00, PIDs publicados 0x1200–0x1203: consulta/inicio, carga por bloques, validación/confirmación y cancelación. No se acepta difusión. Un solo archivo de aplicación (FileID 1); no hay descarga de firmware ni actualización del propio cargador por RDM.

La EEPROM 96 marca una actualización pendiente antes de escribir flash. Las páginas se programan desde el bloque de arranque y se comparan después. El receptor valida CRC16 del paquete/archivo, cabecera de modelo/tamaño y CRC32 de la imagen recibida y grabada. Solo marca la aplicación válida después de un FTC_COMMIT correcto. Los cortes de alimentación, cancelaciones o errores conservan el cargador para reiniciar una transferencia; no se reanuda automáticamente un archivo parcial después de perder alimentación.

No hay doble banco ni copia de la aplicación anterior. Una transferencia interrumpida puede dejar la aplicación inoperativa hasta cargarla otra vez por RDM o USB. El cargador permanece protegido si se instalan correctamente sus bloqueos. La EEPROM de dirección (0–3) y etiqueta (32–64) se conserva. Testmode valida el flujo sin modificar flash ni la marca de validez.

En el arranque normal se ofrece una ventana USB de aproximadamente 1,5 s. Se implementa el subconjunto de STK500v2 usado para programar/verificar aplicación y EEPROM en Mega. No se admite borrado global ni programación de fusibles/bloqueos desde USB. El cargador pone a cero los disparos de pilotos y deshabilita los números; el sketch apaga también las tiras antes de solicitar el cambio a modo de actualización.

La firma de disponibilidad en el sketch impide anunciar FTC cuando sigue instalado el cargador original.

## Programador ArduinoISP con otro Mega

Cargar el ejemplo oficial ArduinoISP en el Mega programador antes de añadir el condensador. Con ambas placas sin alimentación, conectar D50–D50 (MISO), D51–D51 (MOSI), D52–D52 (SCK), D10 del programador a RESET del controlador, GND–GND y 5 V–5 V. Los conectores ICSP pueden quedar vacíos: estas señales son equivalentes. Desconectar el cableado externo del controlador durante la programación, especialmente las salidas que comparten D51/D52. Alimentar únicamente por USB del programador; sin USB ni fuente de barril en el controlador.

En el programador, colocar un electrolítico de 10 µF entre RESET (+) y GND (−) para impedir el reinicio automático al abrir el puerto. Se ha utilizado uno de 100 µF en esta preparación. Retirarlo para volver a cargar sketches por USB en el programador. ArduinoISP utiliza STK500v1 a 19200 baudios; la firma esperada del objetivo ATmega2560 es `1E 98 01`. No forzar una firma incorrecta. El 27 de octubre de 2026 las lecturas erráticas desaparecieron al desconectar el cableado externo del controlador.

### EEPROM con ArduinoISP y avrdude 8

La ruta paginada STK500v1 del ejemplo ArduinoISP mostró una discrepancia de direcciones de EEPROM frente a lecturas USB directas. Para respaldo/restauración utilizar [eeprom_isp.py](../../herramientas/eeprom_isp.py), que envía comandos SPI universales con direcciones de byte y exige firma ATmega2560. La lectura no escribe y la restauración verifica cada byte modificado y la memoria completa. No sustituye el respaldo de flash ni instala el cargador.

```sh
python3 herramientas/eeprom_isp.py --puerto PUERTO_PROGRAMADOR --leer RESPALDO.bin
# Solo si se requiere restaurar un respaldo de 4096 bytes comprobado:
python3 herramientas/eeprom_isp.py --puerto PUERTO_PROGRAMADOR --restaurar RESPALDO.bin
```

La segunda instalación conservó EEPROM durante el borrado ISP programando temporalmente EESAVE (HFUSE `0xD8` a `0xD0`, verificado antes de borrar) y restaurando HFUSE `0xD8` después de verificar flash. No cambiar ningún otro bit ni copiar estos valores en otra placa sin comprobar sus lecturas.
