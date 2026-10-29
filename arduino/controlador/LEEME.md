# Firmware ST-01 v1.0.0

Aplicación Arduino Mega2560, personalidad Control v1.0.0 de 19 canales. [Manual](../../docs/MANUAL_DE_USUARIO.md), [canales](../../docs/MAPA_DE_CANALES_DMX.md), [cableado](../../docs/CABLEADO.md) y [visualizadores](../../docs/VISUALIZADORES.md).

## Compilación y carga

Dependencias: Arduino CLI, Arduino AVR Boards, hd44780 1.3.2 y FastLED 3.9.20.

```sh
arduino-cli compile --fqbn arduino:avr:mega:cpu=atmega2560 --build-path build/controlador arduino/controlador
sh herramientas/cargar_rdm.sh
```

La segunda orden compila, empaqueta y actualiza por RDM. En VS Code, tarea **RDM: compilar y cargar controlador**; carga USB solo mediante la tarea explícita de respaldo. Ver [actualización y recuperación](../../docs/ACTUALIZACION_RDM.md).

## Arquitectura

`digitos.h` convierte códigos DMX en máscaras A–G/DP; `efectos.h` calcula las intensidades RGB; `hardware.h` aplica salidas, cadena SN74HC595N/UDN2981A y tiras. `rdm.h` y `transporte_rdm.h` gestionan RDM/USART1; `ftc.h` comparte el receptor de actualización con el cargador. `version.h` define versión y personalidad; `identidad.h` define UID único por unidad.

No utilizar Serial1 con otra biblioteca. La UART está dedicada al enlace DMX/RDM. Los pines y los grupos WS281x configurados se documentan en el cableado. El diagnóstico USB independiente funciona a 115200 baudios; no es necesario para operación o carga RDM.

Las pruebas automatizadas se ejecutan desde la raíz con `sh pruebas/rdm.sh`. Al cambiar la codificación de canales, mantener alineados firmware, fixture QLC+, workspace, pruebas y manuales.
