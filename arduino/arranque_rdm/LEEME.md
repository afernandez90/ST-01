# Mantenimiento del cargador ST-01

Cargador de 8 KiB para ATmega2560 a 16 MHz, con actualización FTC por RDM y recuperación USB STK500v2. La aplicación v1.0.0 utiliza el mismo cargador; no requiere reinstalarlo al cambiar versión de aplicación.

## Compilación

```sh
sh arduino/arranque_rdm/compilar.sh
```

El guion utiliza AVR GCC instalado con Arduino AVR Boards y produce `build/arranque_rdm/arranque.hex`. No programa la placa ni modifica fusibles. Puede especificarse `ST01_AVR_TOOLS` para otra instalación. `ST01_RS485_MANUAL=1` compila la variante DE+/RE en pin 27; usar la misma definición `-DST01_RS485_MANUAL` al compilar el sketch. La variante predeterminada corresponde al adaptador automático.

El verificador comprueba también que la desactivación watchdog temprana existe en el desensamblado antes de inicializar RAM (retención explícita con LTO), y que todas las direcciones pertenecen a `0x3E000–0x3FFFF`, y que la firma `ST01RDM1` está en `0x3FFF0`. El tamaño máximo de aplicación permanece en 253 952 bytes. El paquete FTC añade una cabecera de 16 bytes que no se graba como código.

## Instalación y servicio ISP

La instalación requiere programador ISP o segundo Mega con ArduinoISP. Antes de borrar: respaldar flash, EEPROM, fusibles y bloqueos; verificar la firma ATmega2560 `1E 98 01`. Conservar la aplicación al reemplazar solo el bloque 0x3E000–0x3FFFF. Configurar región de arranque de 8 KiB y protección frente a SPM sin impedir lectura de firma desde la aplicación. Leer los fusibles reales de cada placa: no copiar valores sin comprobarlos.

Con ArduinoISP, conectar programador D50/D51/D52 a los mismos pines del objetivo, D10 a RESET, GND a GND y 5 V a 5 V. Desconectar cargas y cableado del objetivo y alimentar únicamente desde USB del programador. Añadir condensador de 10–100 µF entre RESET (+) y GND (−) del programador después de cargar ArduinoISP. STK500v1 a 19200 baudios.

Para EEPROM, utilizar `herramientas/eeprom_isp.py --puerto PUERTO --leer RESPALDO.bin`; la ruta SPI universal utiliza direcciones de byte explícitas y evita incompatibilidades de direccionamiento del modo paginado ArduinoISP. La restauración exige exactamente 4096 bytes y verifica la memoria. Para conservar EEPROM durante el borrado, configurar y verificar EESAVE antes de borrar y restaurar después los fusibles originales. Comprobar flash, EEPROM, fusibles y bloqueos al finalizar; verificar carga USB y funcionamiento RDM antes de entregar el equipo.

## Transferencia y recuperación

Se implementan FTC 1.00, PIDs publicados 0x1200–0x1203: consulta/inicio, carga por bloques, validación/confirmación y cancelación. No se acepta difusión. Un solo archivo de aplicación (FileID 1); no hay descarga de firmware ni actualización del propio cargador por RDM.

La EEPROM 96 marca una actualización pendiente antes de escribir flash. Las páginas se programan desde el bloque de arranque y se comparan después. El receptor valida CRC16 del paquete/archivo, cabecera de modelo/tamaño y CRC32 de la imagen recibida y grabada. Solo marca la aplicación válida después de un FTC_COMMIT correcto. Los cortes de alimentación, cancelaciones o errores conservan el cargador para reiniciar una transferencia; no se reanuda automáticamente un archivo parcial después de perder alimentación.

No hay doble banco ni copia de la aplicación anterior. Una transferencia interrumpida puede dejar la aplicación inoperativa hasta cargarla otra vez por RDM o USB. El cargador permanece protegido si se instalan correctamente sus bloqueos. La EEPROM de dirección (0–3) y etiqueta (32–64) se conserva. Testmode valida el flujo sin modificar flash ni la marca de validez.

En el arranque normal se ofrece una ventana USB de aproximadamente 1,5 s. Se implementa el subconjunto de STK500v2 usado para programar/verificar aplicación y EEPROM en Mega. No se admite borrado global ni programación de fusibles/bloqueos desde USB. El cargador pone a cero los disparos de pilotos y deshabilita los números; el sketch apaga también las tiras antes de solicitar el cambio a modo de actualización.

La firma de disponibilidad en el sketch impide anunciar FTC cuando sigue instalado el cargador original.
