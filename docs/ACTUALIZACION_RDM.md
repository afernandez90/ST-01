# Actualización de firmware mediante RDM

## Decisión de interoperabilidad

Se utiliza **FTC 1.00, ANSI E1.37-4-2026**, con PIDs publicados **0x1200–0x1203**. Es un protocolo entre el controlador y ST-01. El cargador se instala en el Mega2560 y no depende de la marca ni del modelo del controlador o interfaz; el controlador/interfaz debe poder transportar RDM y ejecutar esta versión de FTC.

No existe una justificación para instalar un cargador distinto en el Arduino por cada fabricante de consola. Sí se necesitan controladores de transporte diferentes en la aplicación del ordenador para interfaces USB con API distintas.

Comprobación documental del 25 de octubre de 2026:

| Sistema | Documentación comprobada | Implicación para ST-01 |
|---|---|---|
| ESTA | ANSI E1.37-4-2026 publicado; versión FTC 1.00 | Especificación elegida para la implementación |
| ChamSys MagicQ | FTC documentado con PIDs de borrador 0x9202–0x9207 | No asumir compatibilidad con 0x1200–0x1205; pendiente probar una versión que implemente la publicación actual |
| MADRIX RADAR | Notas de versión declaran FTC basado en CP/2017-1024r3, revisión 4.0.056 | Perfil de borrador; interoperabilidad pendiente, no se anuncia como probada |
| ETC | UpdaterAtor actualiza sus equipos mediante RDM y archivos del fabricante | No implica que acepte archivos de ST-01 ni FTC actual |
| MA grandMA3 | Gestión RDM de propiedades documentada | No se ha confirmado un actualizador FTC genérico compatible con ST-01 |
| QLC+ 5.3.0 | Transporte RDM en su complemento, sin un flujo FTC de interfaz verificado | Usar un controlador FTC compatible o la herramienta del proyecto |
| Herramienta ST-01 | Controlador FTC 1.00 con un transporte serie específico de interfaz | Transporte RDM básico probado físicamente; transferencia FTC completa, CRC/duplicados e interrupción de enlace probados físicamente |

No se añaden alias de los PIDs antiguos sin verificar sus campos y revisión del protocolo. El nuevo cargador no modifica ni actualiza el firmware de ningún equipo externo.

El documento publicado se obtuvo desde ESTA tras aceptación expresa de su EULA por el usuario. Se conserva fuera del repositorio. No redistribuir el PDF ni su texto en los entregables. Las implementaciones y las pruebas son código propio.

### Diferencia de longitud en la especificación

La tabla de FTC_INITIATE rotula PDL 0x22, pero la suma de los campos publicados (incluido Max Inter-Packet Delay) es **38 bytes, 0x26**. Se implementan los 38 bytes de los campos descritos. La herramienta verifica esa longitud. Registrar esta discrepancia al validar con controladores reales; no afirmar conformidad o interoperabilidad certificada a partir de la simulación.

## Estado y límites

- Cargador de 8 KiB y sketch compilados para ATmega2560.
- Pruebas de CRC, estados, reintentos, cancelación, límites de flash, recuperación y testmode superadas en simulación.
- Herramienta de empaquetado y secuencia de transferencia probadas en simulación y sobre el controlador real, incluida recuperación tras corte eléctrico.
- **Cargador instalado y verificado por ISP el 27 de octubre de 2026.** Se conservaron aplicación 0.0.7 y EEPROM. La carga/verificación USB STK500v2 y el reinicio se han validado después de corregir los comandos USB; la actualización FTC física y recuperación tras interrupción de enlace o corte eléctrico están validadas; véase el [registro de instalación](INSTALACION_ARRANQUE_RDM.md).
- El sketch detecta la firma del nuevo cargador antes de anunciar los parámetros FTC. Con el cargador original mantiene DMX/RDM básico y las salidas; no ofrece una actualización ficticia.
- No hay doble banco. Una actualización interrumpida requiere volver a cargar la aplicación; el cargador queda disponible para recuperación.
- El retorno RDM básico del DollaTek ya se ha comprobado físicamente. La actualización FTC ya se ha probado físicamente; siguen pendientes medición de tiempos e interoperabilidad con otros controladores. Véanse las [pruebas FTC](PRUEBAS_FTC.md).

## Actualización habitual

Por decisión del usuario, **todas las actualizaciones del controlador se realizan por RDM**. USB e ISP quedan como rutas de instalación del cargador, diagnóstico y recuperación extraordinaria; no se utilizan para la carga habitual de la aplicación.

```sh
sh herramientas/cargar_rdm.sh
```

Este lanzador compila el controlador Mega, empaqueta el HEX recién generado y lo carga por RDM/FTC; después consulta la versión reportada. Requiere Arduino CLI y Python 3 disponibles en el entorno, cargador compatible instalado e interfaz RDM conectada y libre. No requiere USB del Mega. Las variables `ST01_ARDUINO_CLI`, `ST01_PYTHON`, `ST01_RDM_PORT` y `ST01_RDM_UID` permiten elegir herramientas, puerto y equipo. Sin puerto explícito, la utilidad exige una única interfaz detectada; no elige entre varias.

En VS Code, ejecutar **RDM: compilar y cargar controlador**. La plantilla compartida está en `ide/mac/.vscode/tasks.json` y la copia activa en `.vscode/tasks.json`. F5 y la tarea de cargar sketch activo también usan RDM para `controlador`.

## Preparación del firmware

```sh
arduino-cli compile --fqbn arduino:avr:mega:cpu=atmega2560 --build-path build/controlador arduino/controlador
python3 herramientas/firmware_rdm.py empaquetar build/controlador/controlador.ino.hex build/controlador/ST-01.st01
sh arduino/arranque_rdm/compilar.sh
sh pruebas/rdm.sh
```

El paquete ST01 contiene una cabecera propia de 16 bytes (modelo, objetivo Mega2560, longitud y CRC32) seguida de la aplicación binaria. El empaquetador rechaza imágenes que invadan la región de arranque. FTC transporta el archivo sin interpretar su formato; la validación de la imagen corresponde al dispositivo.

## Carga por RDM, después de instalar y validar el cargador

Cerrar las aplicaciones que usen la interfaz. Conectar TX1 (18) y RX1 (19) al adaptador bidireccional correctamente. El controlador y el enlace deben admitir RDM y FTC 1.00.

```sh
python3 herramientas/firmware_rdm.py cargar build/controlador/ST-01.st01 --uid 7ff0:66601337 --puerto PUERTO_RDM
```

La herramienta usa únicamente Python estándar. Esta carga se ha ejecutado correctamente sobre la placa real con la aplicación 0.0.7. Sustituya `PUERTO_RDM` por el puerto de su interfaz. El transporte serie incluido utiliza una API de interfaz concreta, con paquetes delimitados por `0x7E`/`0xE7` y etiquetas de comando; no es una API USB universal. Compruebe su compatibilidad con la interfaz y el acceso serie del equipo. Para otros emisores, utilice un controlador FTC 1.00 compatible o implemente su transporte en la herramienta. Esta limitación de la utilidad de ordenador no condiciona el firmware del Mega.

La secuencia negocia FTC, permite el cambio al cargador, transfiere bloques de 128 bytes con CRC16, respeta las demoras declaradas, comprueba el desplazamiento confirmado, verifica el CRC del archivo antes de FTC_COMMIT y consulta la versión tras reiniciar. Limita los reintentos de paquete. Una interrupción conserva el modo de recuperación; para reiniciar la transferencia se vuelve a ejecutar la carga completa.

La implementación acepta solo FileID 1 y UID individual; no descarga archivos ni actualiza su propio cargador. Se implementa testmode en el receptor, sin escrituras, aunque el comando de carga habitual utiliza transferencia real.

## Validación física

La instalación ISP, carga/verificación USB, actualización RDM, CRC de bloque erróneo, duplicados, desplazamientos fuera de secuencia, interrupción de enlace, corte de alimentación y recuperación, y conservación de dirección/etiqueta están [comprobados físicamente](PRUEBAS_FTC.md). Para repetir la instalación, seguir el [manual del cargador](../arduino/arranque_rdm/LEEME.md) y leer primero la placa objetivo. Pendiente medir tiempos y liberación del bus y probar controladores externos con FTC 1.00 antes de incorporarlos a una lista de compatibilidad confirmada.

Referencias: [publicaciones de ESTA](https://tsp.esta.org/tsp/documents/published_docs.php), [MagicQ y FTC](https://docs.chamsys.co.uk/magicq/1.9.9.x/manual/network_manager.html), [notas MADRIX](https://madrix.com/15-releases), [ETC UpdaterAtor mediante RDM](https://support.etcconnect.com/ETC/Fixtures/ColorSource/PAR/Updating_ColorSource_Par_to_v1.3.0), [gestión RDM de grandMA3](https://help.malighting.com/grandMA3/2.4/HTML/rdm.html), [auto-programación AVR de Microchip](https://developerhelp.microchip.com/xwiki/bin/view/products/mcu-mpu/8-bit-avr/structure/read-write-memory/).

Como respaldo explícito existe la tarea **USB: cargar controlador (respaldo)**: compila, carga por el USB del Mega y abre el monitor serie. Requiere conectar el Mega por USB y cerrar cualquier monitor que ocupe su puerto. La ruta predeterminada y F5 continúan utilizando RDM.
