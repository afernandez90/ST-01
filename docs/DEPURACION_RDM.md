# Registro USB de tráfico RDM

El sketch `recepcion_dmx` registra por USB a **115200 baudios** cada trama RDM recibida y cada respuesta cuya transmisión UART termina. La traza está activada por defecto. Se mantienen los mensajes de configuración y el informe periódico LNB/paquetes por segundo.

## Contenido de cada línea

- `RX` o `TX`, número de evento y tiempo en milisegundos desde el arranque.
- Clase: descubrimiento, GET, SET o respuesta correspondiente.
- Número de transacción, UID de origen y destino y subdispositivo.
- PID hexadecimal con nombre descriptivo, PDL y tipo de respuesta ACK/NACK. Un NACK incluye su código de motivo.
- Resultado: respuesta preparada/transmitida, difusión sin respuesta, otro UID, descubrimiento silenciado o fuera de intervalo, clase no admitida, formato/suma inválidos, cabecera inválida, error UART o recepción incompleta.
- Longitud y **trama completa en hexadecimal**, incluidos cabecera, datos y suma. El contenido binario se representa en hexadecimal para conservar todos los bytes.

Ejemplo abreviado de lectura de dirección:

```text
[RDM RX #15 t=2300ms] GET | trans=8 | origen=454E:02350811 | destino=7FF0:66601337 | sub=0 | PID=0x00F0(DIRECCION_DMX) | PDL=0 | resultado=RESPUESTA_PREPARADA | bytes=26 | trama=...
[RDM TX #16 t=2301ms] RESP_GET | trans=8 | origen=7FF0:66601337 | destino=454E:02350811 | sub=0 | PID=0x00F0(DIRECCION_DMX) | PDL=2 | respuesta=ACK | resultado=TRANSMITIDA | bytes=28 | trama=...
```

Las respuestas de descubrimiento llevan su UID decodificado y los bytes de la respuesta codificada. La dirección destino de una respuesta normal se toma del UID fuente de la solicitud; el registro permite comprobar esa correspondencia.

El instante RX corresponde a la captura de la trama completa o su error; TX se registra cuando ha terminado el último bit de parada. Estos tiempos de milisegundos sirven para correlacionar mensajes y no para medir el break o el retorno eléctrico del bus.

## Recepción incompleta y errores

Un nuevo break interrumpe y registra la trama anterior pendiente. Los errores UART incluyen los bytes capturados y el registro de estado hexadecimal. Una cabecera con subcódigo/longitud inválidos se registra al detectarla. Una trama RDM incompleta se cierra tras 100 ms sin más bytes y se registra como tiempo agotado. Las tramas con suma o PDL incoherentes quedan registradas aunque el respondedor las descarte.

También se registran las solicitudes recibidas dirigidas a otros UIDs, las difusiones y las solicitudes que no producen respuesta. Los paquetes DMX de código de inicio cero conservan su diagnóstico de tasa y LNB, sin volcado individual.

## Cola y rendimiento

Las interrupciones guardan instantáneas en una cola de ocho posiciones (siete utilizables). `loop()` imprime fuera de las interrupciones; la recepción y transmisión UART siguen atendiendo RDM mientras USB envía el texto.

El ancho de banda USB a 115200 baudios limita la cantidad de texto que puede transmitirse. Si una ráfaga llena la cola, la traza muestra explícitamente el número de **eventos de registro** omitidos:

```text
[RDM TRAZA] Cola USB llena; eventos omitidos: 3
```

Cada RX y TX cuenta como un evento. La numeración también avanza para los eventos omitidos, por lo que pueden aparecer saltos. El descarte afecta a la traza; no es un contador de paquetes RDM perdidos. Para una captura textual completa, espaciar las consultas o reducir el tráfico del controlador cuando aparezca este aviso.

## Comprobaciones realizadas

25 de octubre de 2026, Mega2560 y controlador de pruebas DMX/RDM:

- Prueba RDM completa con consultas espaciadas: captura de RX/TX, UIDs, descubrimiento codificado, silencio, ACK y NACK de dirección fuera de rango; sin eventos de traza omitidos.
- Ráfaga de 24 lecturas de dirección: las 24 recibieron respuesta correcta; se observaron los avisos explícitos de saturación de la traza.
- Dirección, etiqueta e identificación originales restauradas al terminar.
- Pruebas de simulación: captura sin imprimir desde interrupciones, instantánea independiente del búfer RX, errores de formato/suma, cabecera inválida, corte por break, tiempo agotado, error UART, NACK, descubrimiento y cola llena.

## Alcance del cargador

Este registro textual pertenece al **sketch de aplicación**. El cargador FTC de 8 KiB conserva su interfaz USB binaria STK500v2 y su implementación actual no produce esta traza textual. Las solicitudes FTC que lleguen mientras se ejecuta la aplicación sí aparecen en el registro de la aplicación; antes de reiniciar se detiene la recepción y se vacía la traza pendiente de la última respuesta; después de cambiar al cargador, la observación corresponde al controlador de actualización.

El código del registro está en `arduino/pruebas/recepcion_dmx/traza_rdm.h`. Las pruebas se ejecutan con `sh pruebas/rdm.sh`. El monitor habitual de VS Code continúa usando 115200 baudios.
