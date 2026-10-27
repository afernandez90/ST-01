# Firmware Arduino de ST-01

`controlador.ino` es el sketch principal para Arduino Mega 2560. Implementa la personalidad DMX de 19 canales `Control v0.0.9`, documentada en el [mapa de canales](../../docs/MAPA_DE_CANALES_DMX.md), con recepción DMX, gestión RDM, efectos LED, salidas y controles locales.

## Compilación

```sh
arduino-cli compile --fqbn arduino:avr:mega:cpu=atmega2560 --build-path build/controlador arduino/controlador
```

Instale las bibliotecas `hd44780@1.3.2` y `FastLED@3.9.20` con Arduino CLI. La carpeta y el sketch comparten el nombre `controlador`, como requiere Arduino. Para cargar o abrir el monitor, seleccione el puerto detectado por Arduino CLI o utilice las tareas de desarrollo configuradas para su entorno.

La reorganización no cambia la personalidad ni el comportamiento del firmware. Las salidas físicas y el cargador FTC conservan su estado de validación documentado en [continuidad](../../docs/CONTINUIDAD.md). No cambie los desplazamientos sin actualizar la personalidad, el fixture y la documentación.

## Recepción DMX y controles locales

El controlador recibe DMX por RX1 (pin 19) a 250 kbaud, 8N2, mediante el enlace RS-485. TX1 (18) proporciona el retorno RDM. El LED integrado reproduce el piloto ámbar de Cara B (desplazamiento 13) y se apaga tras un segundo sin datos. Los sketches independientes `parpadeo` y `escaner_i2c` permanecen en [`../pruebas/`](../pruebas/).

El LCD muestra la dirección base pendiente en la primera fila; el asterisco indica cambios sin guardar. La segunda fila muestra DMX OK/SIN DMX y `LNB` (LED Naranja Cara B) con el valor recibido (0–255), sin número de canal:

```text
ST-01 DMX: 001
DMX OK  LNB:  0
```

Sin señal, se conserva la última lectura en pantalla y la salida se apaga tras un segundo.

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
| 22 | Dirección −10 |
| 23 | Dirección −1 |
| 24 | Dirección +1 |
| 25 | Dirección +10 |
| 26 | Guardar dirección en EEPROM |

Los botones editan una dirección pendiente que se muestra en el LCD con un asterisco. La recepción DMX mantiene la dirección activa hasta pulsar Guardar (pin 26), que aplica la nueva dirección y la guarda en EEPROM. Reiniciar sin guardar descarta los cambios pendientes. Al reiniciar se recupera la dirección guardada; sin configuración válida se utiliza 1. El rango es 1–494 para reservar los 19 canales de ST-01. Cada pulsación genera un paso; no hay repetición al mantener pulsado.

El LED L integrado reproduce el piloto ámbar de Cara B: desplazamiento 13, canal absoluto `direccionBase + 12` (canal 13 con dirección base 1). No depende de un regulador maestro del dispositivo. Si cambia la dirección del equipo, hay que cambiar también la dirección de la instancia ST-01 en QLC+ o probar el canal correspondiente desde Simple Desk.

No utilizar `Serial1` en esta prueba: la recepción y las respuestas RDM utilizan directamente las interrupciones USART1.

## Respondedor RDM

La prueba incorpora descubrimiento RDM, información del equipo, dirección remota (1–494), etiqueta persistente e identificación con parpadeo de todos los pilotos, números, tiras, LED L e iluminación del LCD. Mantiene los 19 canales, los botones y el comportamiento LNB habitual al desactivar la identificación. El sketch se ha cargado y se ha [comprobado físicamente el retorno RDM básico](../../docs/PRUEBAS_RDM.md); las salidas finales y el cargador FTC continúan pendientes de validación. Consulte [conexionado, comandos y validación RDM](../../docs/RDM.md).

## Diagnóstico USB de recepción DMX

A 115200 baudios, el firmware indica la dirección activa y el pin RX1 (19) al arrancar, y notifica cada cambio de dirección. Aproximadamente una vez por segundo muestra:

```text
DMX OK | Piloto ambar B: 239 | Paquetes/s: 40.0
```

La tasa corresponde a paquetes válidos que contienen el canal del piloto ámbar B de la dirección activa, divididos por el tiempo real de la ventana de medición. No es un contador acumulado. Sin señal, el valor del piloto ámbar B conserva la última lectura, como el LCD; la salida PWM se apaga tras un segundo sin datos. Las direcciones pendientes se notifican al editarlas, separadas de los informes periódicos y de la dirección activa.

Los avisos de inicio, EEPROM y LCD, los cambios de dirección pendiente o guardada y las transiciones de recepción/pérdida de DMX se envían por el mismo puerto USB. La dirección y su estado de guardado siguen funcionando como antes.

## Salidas y actualización

El [cableado elegido](../../docs/CABLEADO_PROPUESTO.md) asigna los pines y cuatro TPIC6B595 a los números. Instalar también `arduino-cli lib install "FastLED@3.9.20"`. El firmware controla las 19 funciones; las cantidades de píxeles y el montaje son provisionales. La [actualización mediante FTC](../../docs/ACTUALIZACION_RDM.md) necesita instalar una vez el cargador mediante ISP; no está disponible con el cargador original. Las pruebas simuladas no sustituyen las pruebas eléctricas.

Al arrancar o reiniciar, el controlador realiza una autocomprobación visual de un segundo: dos destellos de 250 ms separados por pausas de 250 ms, con el mismo patrón de todas las salidas que la identificación RDM (pilotos, segmentos incluido el punto, ambas tiras en blanco, LED de estado e iluminación del LCD). No requiere señal DMX ni una petición RDM. Durante la prueba sigue recibiendo DMX y respondiendo a RDM; al terminar aplica el estado DMX vigente, o apaga las salidas si no hay señal válida. La prueba no modifica el estado RDM de identificación. Es una comprobación visual, sin detección automática de averías.
