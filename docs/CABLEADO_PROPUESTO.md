# Cableado decidido para el prototipo

Asignación elegida para implementar todas las salidas. **Validación parcial:** el usuario ha confirmado el funcionamiento del piloto rojo con fuente de 12 V y módulo MOSFET el 26 de octubre de 2026. El resto del montaje sigue pendiente de comprobación. La Barra tiene 38 píxeles comprobados; el recuento de la Tira y las características eléctricas de los números siguen pendientes de medida.

| Salida o entrada | Pin Mega | Interfaz |
|---|---:|---|
| Piloto azul A | 2 | PWM → entrada del módulo MOSFET JZK |
| Piloto verde A | 3 | PWM → JZK |
| Piloto naranja A | 4 | PWM → JZK |
| Piloto rojo A | 5 | PWM → JZK |
| Piloto naranja B / LNB | 6 | PWM → JZK |
| Piloto blanco izquierdo B | 7 | PWM → JZK |
| Piloto blanco derecho B | 8 | PWM → JZK |
| Piloto azul de estado B | 9 | Salida digital → JZK adicional, si es un piloto de 12 V |
| Datos Barra WS2812B | 10 | DIN, alimentación externa de 5 V |
| Datos Tira WS2811 | 11 | DIN, alimentación externa de 12 V |
| LED L integrado | 13 | Copia de LNB / identificación |
| Transmisión RDM | 18 / TX1 | Entrada TTL de transmisión del adaptador RS-485 |
| Recepción DMX/RDM | 19 / RX1 | Salida TTL de recepción del adaptador |
| LCD SDA/SCL | 20 / 21 | PCF8574 en 0x27 |
| Botones | 22–26 | 22: −10, 23: −1, 24: +1, 25: +10, 26: Guardar; KCOM a GND |
| Dirección RS-485 manual opcional | 27 | DE y /RE unidos; sin uso en el DollaTek automático |
| Habilitación números | 48 | /G de los cuatro TPIC6B595 |
| Retención números | 49 | RCK común |
| Datos números | 51 | SER IN del primer TPIC6B595 |
| Reloj números | 52 | SRCK común |

## Números

El usuario confirma visualizadores **50101AS**, rojos, de **5 pulgadas**, **un dígito y ocho segmentos**, de **cátodo común**. Falta verificar patillaje, tensión y corriente por segmento. El pequeño seleccionado para las pruebas es **CL5611AH de 0,56 pulgadas**. Las [referencias de fabricante](REFERENCIAS_VISUALIZADORES.md) muestran un patillaje distinto del ELS-50101AS grande y tensiones de segmento muy diferentes. Al recibir los grandes se repetirán las medidas y se adaptarán el arnés, la alimentación y las resistencias, incluido el punto decimal. La etapa candidata utiliza 74HC595 y TBD62783APG; no se considera validada hasta probarla.

**Etapa de números pendiente de rediseño:** la propuesta anterior usa **cuatro TPIC6B595**, uno por dígito, sin multiplexación. Son salidas de drenador abierto que conectan los segmentos a masa; este diseño requiere dígitos de ánodo común y limitación de corriente por segmento. Por tanto, esta conexión directa no es compatible con los visualizadores 50101AS de cátodo común confirmados. Se necesita una etapa que suministre corriente a los segmentos; seleccionar el circuito tras verificar la ficha técnica. No alimentar segmentos desde los pines del Mega.

Las conexiones siguientes documentan la propuesta anterior y no deben montarse como circuito definitivo:

- Primer circuito, cercano al Mega: millares; después centenas, decenas y unidades. SER OUT de cada circuito va a SER IN del siguiente.
- Alimentación lógica de los TPIC a 5 V y masa común. /SRCLR fijo a HIGH. /G, RCK y SRCK compartidos.
- DRAIN0–7: A, B, C, D, E, F, G y punto decimal. Un bit 1 enciende el segmento correspondiente.
- Resistencia de 10 kΩ de /G a 5 V para mantener los números apagados mientras el Mega arranca o se programa por ISP.
- Elegir alimentación, resistencias y corriente de los segmentos según el número físico. TI especifica 150 mA por salida para el TPIC6B595; comprobar también los límites térmicos del encapsulado y el circuito. Si los números adquiridos no son de ánodo común o exceden los límites, adaptar la etapa de potencia conservando los mismos pines lógicos.

La codificación DMX existente no cambia. Identificar muestra `8.` en los cuatro números y alterna con apagado completo.

## Pilotos y tiras

El montaje del piloto rojo con fuente de 12 V y MOSFET ha sido probado por el usuario y se adopta como referencia para los demás pilotos. La disposición exacta de los terminales de potencia del módulo sigue pendiente de documentar; conservar el conexionado verificado. No se ha medido todavía la curva de regulación PWM.

Los siete pilotos DMX son de 12 V. Cada uno utiliza un módulo JZK; el octavo se reserva para el piloto de estado si también es de 12 V. Conectar los pines indicados a las entradas lógicas de disparo de los módulos. Las cargas toman su corriente de la fuente de 12 V. Añadir una resistencia de 10 kΩ a masa en cada disparo para mantenerlo apagado durante el reinicio.

Las tiras usan puertos de datos distintos y sus fuentes correspondientes, con masa común. Configuración inicial elegida en `hardware.h`:

- Barra: WS2812B, orden GRB, **38 píxeles**, comprobados con progreso manual el 26 de octubre de 2026.
- Tira: WS2811 a 800 kHz, orden RGB, **300 píxeles direccionables**. En la COB, un píxel es un circuito/grupo controlable, no cada LED físico.
- Brillo RGB limitado a 64/255 durante la puesta en marcha.

El recuento de la Barra está confirmado; el de la Tira sigue siendo provisional. Contar sus grupos reales y ajustar las constantes antes de la validación final. Si hay más grupos que los configurados, el exceso no recibirá la orden de identificación. Comprobar el orden RGB con una prueba de rojo, verde y azul. La identificación usa blanco en ambas tiras.

FastLED 3.9.20 mantiene habilitadas las interrupciones en AVR para conservar la recepción DMX/RDM. Sus controladores WS281x son sensibles a interrupciones largas: las respuestas RDM pueden perturbar un fotograma de píxeles. El firmware repinta cada 50 ms, pero queda pendiente verificar con las tiras reales que la señal y los colores son estables. No presentar esta temporización como validada físicamente.

## Identificación

`IDENTIFY_DEVICE = 1` toma prioridad sobre DMX y sobre su pérdida: todos los pilotos, ambos grupos RGB, los cuatro números y el LED L parpadean juntos a 2 Hz. La iluminación del LCD sigue el mismo patrón y su primera línea indica `IDENTIFICAR ST01`. Al desactivar identificar, vuelven los valores DMX vigentes; sin señal válida, las salidas se apagan. La identificación no modifica el mapa de canales ni las escenas.

Referencias: [TPIC6B595 de TI](https://www.ti.com/product/TPIC6B595), [temporización AVR de FastLED](https://github.com/FastLED/FastLED/blob/3.9.20/src/platforms/avr/clockless_trinket.h). El esquema final y el recorrido físico se prepararán tras comprobar el montaje, como quedó acordado.
