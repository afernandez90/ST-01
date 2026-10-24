# Pruebas de hardware

Placa: Arduino Mega 2560 (`arduino:avr:mega:cpu=atmega2560`).

- `parpadeo`: prueba visual de carga mediante el LED integrado.
- `escaner_i2c`: busca dispositivos en SDA (20) y SCL (21); informa por USB a 115200 baudios.
- `recepcion_dmx`: recibe DMX por RX1 (19), regula el LED integrado con el primer canal de la dirección seleccionada y muestra dirección, estado y valor en el LCD de 16×2. Apaga el LED tras un segundo sin datos.

## Pantalla LCD

El adaptador PCF8574 respondió en **0x27** durante la prueba física. Conexiones: VCC a 5V, GND a GND, SDA a 20 y SCL a 21.

La prueba de recepción utiliza la biblioteca **hd44780 1.3.2**, que configura automáticamente las conexiones internas del adaptador:

```sh
arduino-cli lib install "hd44780@1.3.2"
```

## Botones y dirección DMX

KCOM se conecta a GND. Las cinco entradas utilizan `INPUT_PULLUP` y antirrebote de 30 ms:

| Pin Mega | Función |
|---|---|
| 22 | Dirección −1 |
| 23 | Dirección +1 |
| 24 | Dirección −10 |
| 25 | Dirección +10 |
| 26 | Guardar dirección en EEPROM |

Los botones editan una dirección pendiente que se muestra en el LCD con un asterisco. La recepción DMX mantiene la dirección activa hasta pulsar Guardar (pin 26), que aplica la nueva dirección y la guarda en EEPROM. Reiniciar sin guardar descarta los cambios pendientes. Al reiniciar se recupera la dirección guardada; sin configuración válida se utiliza 1. El rango es 1–493 para reservar los 20 canales de ST-01. Cada pulsación genera un paso; no hay repetición al mantener pulsado.

El LED integrado responde al primer canal de la dirección seleccionada. Si cambia la dirección del equipo, hay que cambiar también la dirección de la instancia ST-01 en QLC+ o probar el canal correspondiente desde Simple Desk.

No utilizar `Serial1` en esta prueba: la recepción utiliza directamente su interrupción USART1.
