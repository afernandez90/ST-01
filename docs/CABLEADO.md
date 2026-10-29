# Conexiones ST-01 v1.0.0

Arduino Mega2560. Montar sin alimentación y compartir masa entre controlador, interfaces y fuentes de las cargas. Las cargas se alimentan desde sus fuentes externas; no alimentar tiras ni segmentos desde GPIO.

| Pin Mega | Conexión |
|---:|---|
| 2 | MOSFET del piloto azul |
| 3 | MOSFET del piloto verde |
| 4 | MOSFET del piloto naranja A |
| 5 | MOSFET del piloto rojo |
| 6 | MOSFET del piloto naranja B |
| 7 | MOSFET del piloto blanco izquierdo |
| 8 | MOSFET del piloto blanco derecho |
| 9 | Módulo del piloto de estado, si utiliza carga externa |
| 10 | DIN Barra WS2812B, 38 píxeles, orden GRB |
| 11 | DIN Tira WS2811, orden RGB; firmware configurado para 300 grupos direccionables |
| 13 | LED integrado, copia del piloto naranja B |
| 18 / TX1 | Entrada TTL de transmisión del adaptador RS-485 |
| 19 / RX1 | Salida TTL de recepción del adaptador RS-485 |
| 20 / SDA | SDA del LCD PCF8574 |
| 21 / SCL | SCL del LCD PCF8574 |
| 22, 23, 24, 25 | Botones −10, −1, +1, +10 |
| 26 | Botón Guardar |
| 48 | /OE pin 13 de los cuatro SN74HC595N |
| 49 | Latch pin 12 de los cuatro SN74HC595N |
| 51 | Datos pin 14 del primer SN74HC595N |
| 52 | Reloj pin 11 de los cuatro SN74HC595N |

## Fuentes

Mega: 12 V por barril. Pilotos: fuente de 12 V a través de los módulos MOSFET. Barra: fuente externa de **5 V**, nunca 12 V. Tira: fuente correspondiente a su modelo WS2811; la variante prevista utiliza 12 V. Verificar tensión nominal y contar grupos direccionables, no LED físicos, antes de conectar otra tira. Números: [circuito completo para pequeño y grande](VISUALIZADORES.md).

Mantener separadas las ramas positivas. Si el Mega está alimentado por USB o barril, no conectar una fuente externa adicional al pin 5 V; compartir únicamente masa con la fuente de la carga.

## Pilotos

Para cada módulo MOSFET: entrada de alimentación a su fuente, salida de carga al piloto según las marcas de polaridad, TRIG/PWM al pin Mega de la tabla y GND lógico a masa común. Añadir 10 kΩ de cada entrada de disparo a masa. No deducir posiciones físicas de bornes de otro modelo de módulo. Pilotos completos nominales de 12 V; no sustituir por LED desnudos sin limitación de corriente.

## Tiras

Conectar el pin de datos a DIN, en el extremo de entrada indicado por las flechas. Una resistencia de 330–470 Ω en serie con datos cerca del Mega. Positivo de la tira a su fuente, negativo a masa común. El límite global de brillo del firmware es 64/255. El refresco es de 50 ms y las interrupciones permanecen habilitadas para atender DMX/RDM.

## LCD y botones

LCD I2C 16×2, dirección 0x27, VCC 5 V y GND común. Botones normalmente abiertos entre su entrada y GND; si comparten KCOM, KCOM va a GND. Se utilizan pull-ups internos. Guardar aplica la dirección pendiente y la conserva entre reinicios.

## Enlace DMX/RDM

Usar transceptor RS-485 bidireccional con retorno RDM, alimentación adecuada y masa de referencia. En la configuración estándar el adaptador controla automáticamente el sentido. Para un transceptor manual: RO a 19, DI a 18, DE y /RE unidos a 27; requiere compilar aplicación y cargador con la misma selección de dirección manual. Respetar la polaridad DMX y terminar el extremo de la línea según la instalación.
