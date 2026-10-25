# Cargador RDM del Arduino Mega 2560

Cargador experimental de 8 KiB para ATmega2560 a 16 MHz. La misma imagen sirve independientemente de la marca del controlador RDM: el protocolo reside en el Arduino. No actualiza el firmware de ENTTEC, MA, ChamSys ni otros equipos externos.

**Estado: compilado y probado mediante simulación del receptor FTC; no instalado ni probado físicamente. El usuario no dispone actualmente de un programador ISP.** El manejo USB STK500v2 está implementado, pero su interoperabilidad con avrdude y el reinicio real siguen pendientes de prueba en placa.

## Compilación

```sh
sh arduino/arranque_rdm/compilar.sh
```

El guion utiliza AVR GCC instalado con Arduino AVR Boards y produce `build/arranque_rdm/arranque.hex`. No programa la placa ni modifica fusibles. Puede especificarse `ST01_AVR_TOOLS` para otra instalación. `ST01_RS485_MANUAL=1` compila la variante DE+/RE en pin 27; usar la misma definición `-DST01_RS485_MANUAL` al compilar el sketch. La variante predeterminada corresponde al adaptador automático.

El verificador comprueba que todas las direcciones pertenecen a `0x3E000–0x3FFFF`, y que la firma `ST01RDM1` está en `0x3FFF0`. El tamaño máximo de aplicación permanece en 253 952 bytes. El paquete FTC añade una cabecera de 16 bytes que no se graba como código.

## Instalación inicial pendiente

Requiere un programador ISP o un segundo Arduino con ArduinoISP. No es una carga normal de sketch por USB. Antes de instalar:

1. Leer y conservar la firma del chip, flash completa, EEPROM, fusibles y bloqueos actuales mediante el programador elegido.
2. Preparar una imagen completa que conserve la aplicación y reemplace únicamente los 8 KiB del cargador; conservar la EEPROM. Un borrado ISP del chip puede eliminar la aplicación y la EEPROM según los fusibles.
3. Configurar arranque en el bloque de 8 KiB. Proteger el cargador frente a SPM, permitiendo su lectura desde la aplicación para detectar la firma. Verificar los valores contra la hoja de datos del ATmega2560 y las lecturas reales antes de programarlos; no copiar fusibles de otra placa.
4. Programar y verificar flash/EEPROM/fusibles, comprobar carga y verificación USB, reinicio normal y recuperación RDM tras una interrupción de transferencia.
5. Validar también el retorno RS-485 automático. La recepción DMX ya comprobada no confirma que el DollaTek transmita y libere correctamente el bus.

No se incluye un comando de instalación con fusibles supuestos. La instalación se concretará cuando esté disponible el programador y puedan obtenerse sus lecturas reales.

## Transferencia y recuperación

Se implementan FTC 1.00, PIDs publicados 0x1200–0x1203: consulta/inicio, carga por bloques, validación/confirmación y cancelación. No se acepta difusión. Un solo archivo de aplicación (FileID 1); no hay descarga de firmware ni actualización del propio cargador por RDM.

La EEPROM 96 marca una actualización pendiente antes de escribir flash. Las páginas se programan desde el bloque de arranque y se comparan después. El receptor valida CRC16 del paquete/archivo, cabecera de modelo/tamaño y CRC32 de la imagen recibida y grabada. Solo marca la aplicación válida después de un FTC_COMMIT correcto. Los cortes de alimentación, cancelaciones o errores conservan el cargador para reiniciar una transferencia; no se reanuda automáticamente un archivo parcial después de perder alimentación.

No hay doble banco ni copia de la aplicación anterior. Una transferencia interrumpida puede dejar la aplicación inoperativa hasta cargarla otra vez por RDM o USB. El cargador permanece protegido si se instalan correctamente sus bloqueos. La EEPROM de dirección (0–3) y etiqueta (32–64) se conserva. Testmode valida el flujo sin modificar flash ni la marca de validez.

En el arranque normal se ofrece una ventana USB de aproximadamente 1,5 s. Se implementa el subconjunto de STK500v2 usado para programar/verificar aplicación y EEPROM en Mega. No se admite borrado global ni programación de fusibles/bloqueos desde USB. El cargador pone a cero los disparos de pilotos y deshabilita los números; el sketch apaga también las tiras antes de solicitar el cambio a modo de actualización.

La firma de disponibilidad en el sketch impide anunciar FTC cuando sigue instalado el cargador original.
