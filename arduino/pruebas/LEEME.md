# Pruebas de hardware

Placa: Arduino Mega 2560 (`arduino:avr:mega:cpu=atmega2560`).

- `parpadeo`: prueba visual de carga mediante el LED integrado.
- `escaner_i2c`: busca dispositivos en SDA (20) y SCL (21); informa por USB a 115200 baudios.

El firmware principal se encuentra en [`../controlador/`](../controlador/). Su documentación incluye recepción DMX, LCD, botones, salidas, RDM y actualización FTC. Esta carpeta conserva únicamente las pruebas independientes de hardware.
