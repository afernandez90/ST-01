# RDM en el prototipo ST-01

El sketch `arduino/pruebas/recepcion_dmx/recepcion_dmx.ino` incorpora un respondedor RDM experimental. Conserva la personalidad `Control v0.0.6` de 19 canales, la dirección base 1–494, los botones, el LCD y la salida de prueba del piloto naranja B en el LED L.

Estado: compilado y cargado en Mega2560; [RDM básico validado físicamente](PRUEBAS_RDM.md) con ENTTEC 2.4 y el DollaTek. También se conservan las pruebas de tramas y UART simuladas. No constituye una implementación certificada de E1.20. El nuevo cargador FTC sigue pendiente de instalación por ISP.

## Conexión de retorno

El adaptador indicado por el usuario es [DollaTek B07DJ4TGY3](https://www.amazon.co.uk/dp/B07DJ4TGY3), anunciado como convertidor TTL/RS-485 con dirección automática. No se dispone de un esquema del fabricante ni de especificaciones de su tiempo de retorno: las pruebas han confirmado el intercambio RDM básico, pero no sustituyen la medición eléctrica de su temporización.

- Conservar la salida TTL de recepción del adaptador hacia **RX1, pin 19** del Mega.
- Conectar **TX1, pin 18** del Mega a la entrada TTL de transmisión del adaptador.
- Comprobar el sentido real de los pines TXD/RXD del módulo: algunas placas los rotulan desde la perspectiva del anfitrión. No unir dos salidas.
- Mantener alimentación, masa y conexiones A+/B− ya utilizadas. No inferir aislamiento galvánico de este módulo.
- La configuración predeterminada `DIRECCION_RS485_AUTOMATICA = true` en `transporte_rdm.h` no utiliza un GPIO de dirección.

Si el adaptador automático no transmite correctamente el break o tarda demasiado en soltar el bus, sustituirlo por un transceptor con control explícito: RO→19, DI→18, DE y /RE unidos→27, y configurar `DIRECCION_RS485_AUTOMATICA = false`. Estas conexiones corresponden a un transceptor manual, no al DollaTek sin DE/RE expuestos.

En el prototipo la UART espera 200 µs antes de responder. Las respuestas normales generan un break de 180 µs y una marca de al menos 40 µs, usando un cero a 50 kbaud; el resto se transmite a 250 kbaud, 8N2. Descubrimiento transmite la respuesta codificada sin break. La transmisión usa interrupciones y vuelve a recepción después del último bit de parada. Confirmar los tiempos **en A/B**, porque el adaptador automático puede modificar la señal.

## Identidad y comandos

UID del prototipo único: **7FF0:66601337**, definido de forma común para el respondedor y FTC en `identidad.h`. `7FF0` pertenece al rango experimental de ESTA; no es un identificador comercial de Salto Temporal. Antes de utilizar otro ejemplar en el mismo bus, asignarle un número de dispositivo diferente en los últimos cuatro bytes en `identidad.h`; el diagnóstico de inicio se genera automáticamente. El UID no depende de la dirección DMX ni cambia al reiniciar.

| Parámetro | Operaciones y resultado |
|---|---|
| DISC_UNIQUE_BRANCH | Descubrimiento dentro de límites inclusivos |
| DISC_MUTE / DISC_UN_MUTE | Silenciar/restablecer las respuestas de descubrimiento |
| SUPPORTED_PARAMETERS | Parámetros adicionales admitidos |
| DEVICE_INFO | Modelo 1, versión de protocolo 1.0, firmware 0.0.6, 19 canales, una personalidad, sin subdispositivos ni sensores |
| DEVICE_MODEL_DESCRIPTION | ST-01 Control v0.0.6 |
| MANUFACTURER_LABEL | Salto Temporal |
| DEVICE_LABEL | GET/SET, hasta 32 bytes, persistente |
| SOFTWARE_VERSION_LABEL | 0.0.6 RDM prototipo |
| DMX_PERSONALITY | GET/SET; solo se admite personalidad 1 |
| DMX_PERSONALITY_DESCRIPTION | Control v0.0.6, 19 canales |
| DMX_START_ADDRESS | GET/SET, 1–494, persistente |
| IDENTIFY_DEVICE | GET/SET, 0/1; parpadeo conjunto de todas las salidas a 2 Hz |

Identificar sustituye temporalmente todas las salidas: los cuatro números muestran 8. (todos los segmentos), los siete pilotos, el piloto de estado, las dos tiras RGB en blanco, el LED L y la iluminación del LCD parpadean juntos a 2 Hz. Al desactivarlo se recupera DMX; sin señal las salidas se apagan. No modifica la lectura LNB ni la medición de paquetes/s. Véase el [cableado elegido y las cantidades de diseño](CABLEADO_PROPUESTO.md); la respuesta física depende de completar y comprobar ese montaje. Identificar y el silencio de descubrimiento se restablecen al reiniciar.

Un cambio remoto de dirección se aplica antes del siguiente paquete DMX, reemplaza la dirección pendiente de los botones y genera un aviso USB. La EEPROM conserva la estructura existente en 0–3; la marca y etiqueta RDM utilizan 32–64. Las escrituras y mensajes USB se ejecutan fuera de las interrupciones, después de preparar la respuesta. Un corte de alimentación inmediato puede interrumpir la persistencia. La dirección activa no cambia al editar botones hasta pulsar Guardar.

Se comprueban código de inicio, subcódigo, longitud, PDL, destino y suma antes de aplicar comandos. Se contestan errores de parámetro, formato, rango, clase de comando y subdispositivo. Las difusiones generales y del fabricante aplican SET sin respuesta; GET por difusión se ignora. No se implementan sensores ni control de intensidades por RDM. Se ha añadido la ruta [FTC de actualización de firmware](ACTUALIZACION_RDM.md), anunciada solo al detectar el cargador compatible. Su instalación inicial por ISP y la validación física siguen pendientes.

## Verificación

Pruebas locales sin placa:

```sh
sh pruebas/rdm.sh
arduino-cli compile --fqbn arduino:avr:mega:cpu=atmega2560 arduino/pruebas/recepcion_dmx
```

El guion usa el compilador C++ del sistema y sanitizadores de memoria/comportamiento indefinido. Comprueba descubrimiento y su suma codificada, extremos inclusivos, silencio, GET/SET, difusiones, NACK, direcciones límite, etiquetas, tramas truncadas/incoherentes y 20 000 entradas arbitrarias. La simulación del sketch comprueba la secuencia de UART, el retorno a RX, la dirección persistente, el LCD, la identificación y el apagado por pérdida de DMX. No verifica tiempos físicos ni el adaptador.

Para ampliar o repetir la prueba física (la primera ronda está en [resultados](PRUEBAS_RDM.md)):

1. Confirmar el conexionado de transmisión y cargar el sketch con la tarea habitual de VS Code; el monitor sigue a 115200 baudios.
2. Usar un controlador RDM y ENTTEC con firmware DMX/RDM 2.4. Se ha confirmado por lectura directa el firmware 2.4 y se ha restaurado su frecuencia de salida de 1 a 40 paquetes/s. QLC+ 5.3.0 no ofrece un flujo de gestión RDM completo verificado en su interfaz actual.
3. Descubrir el UID y leer información, dirección y personalidad. Comprobar silencio y nuevo descubrimiento tras desilenciar.
4. Cambiar la dirección, comprobar LCD/USB y el canal LNB en la nueva dirección; reiniciar y confirmar persistencia. Rechazar 0 y 495. Restaurar la dirección de trabajo.
5. Activar/desactivar identificar; probar etiqueta y persistencia. Mantener DMX normal, apagar su envío y comprobar el apagado tras un segundo.
6. Medir con osciloscopio/analizador la respuesta y liberación del bus, incluido descubrimiento. Si falla la dirección automática, emplear el transceptor manual descrito arriba.

Referencias: [E1.20 publicado por ESTA](https://tsp.esta.org/tsp/documents/docs/E1-20RDM_2006.pdf), [identificadores de fabricante y rango experimental](https://tsp.esta.org/tsp/working_groups/CP/mfctrIDs.php), [API ENTTEC](https://cdn.enttec.com/pdf/assets/70304/70304_DMX_USB_PRO_API.pdf).

## Traza detallada por USB

El sketch registra cada solicitud y respuesta RDM con UID, transacción, PID, ACK/NACK y bytes completos, además de tramas inválidas o incompletas. Véase [depuración RDM](DEPURACION_RDM.md) para los campos, ejemplos, límites de la cola y resultados de la prueba física. La impresión se ejecuta fuera de las interrupciones.
