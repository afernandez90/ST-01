# Visualizadores de siete segmentos — ST-01 v1.0.0

Cada dígito es estático, de cátodo común, con siete segmentos A–G y punto decimal. La unidad de cuatro dígitos utiliza **cuatro SN74HC595N y cuatro UDN2981A**, uno de cada por dígito. No hay multiplexación. No conectar los LED directamente al Mega ni usar drivers de sumidero como sustitutos del UDN.

## Variantes y alimentación

| Parámetro | Pequeño | Grande |
|---|---|---|
| Referencia | CL5611AH, rojo, 0,56 pulgadas | 50101AS rojo, 5 pulgadas; patillaje de referencia ELS-50101AS |
| Polaridad | Cátodo común | Cátodo común |
| SN74HC595N y lógica Mega | 5 V | 5 V |
| UDN2981A y segmentos | 5 V | Rail independiente de 18 V |
| Resistencias A–G | 330 Ω por segmento | 560 Ω / 1 W por segmento, punto de partida conservador |
| Resistencia DP | 330 Ω | 1,5 kΩ / 1 W, punto de partida conservador |
| Estado de comprobación | Primer dígito completo comprobado físicamente | Diseño basado en ficha de referencia; comprobar las unidades reales antes de energizar |

La referencia comercial 50101AS no identifica por sí sola un fabricante. Antes de montar los grandes, verificar que la unidad corresponde al patillaje y polaridad de esta tabla. La ficha Etnal publica VF de 12,6–14 V para A–G y 5,4–6 V para DP, a 20 mA de referencia. No son tensiones de alimentación ni un límite de corriente de diseño. Los grandes no se conectan al arnés pequeño sin cambiar conexiones y resistencias.

Los valores de resistencia de 18 V son una configuración inicial limitada en corriente, no una garantía de brillo o corriente para cualquier 50101AS. El UDN tiene una caída propia dependiente de la carga. Medir corriente por segmento y temperatura antes de fijar el montaje definitivo: `I = Vresistencia / R`; `R = (Vfuente − Vdriver − VF) / I`. No reducir resistencias sin medir. Los 330 Ω a 5 V se han comprobado con el CL5611AH.

## Numeración de los chips

Vista superior, muesca a la izquierda. No confundir pines con números del protoboard.

```text
SN74HC595N:
       16 15 14 13 12 11 10  9
      (                      |
        1  2  3  4  5  6  7  8

UDN2981A:
       18 17 16 15 14 13 12 11 10
      (                         |
        1  2  3  4  5  6  7  8  9
```

**SN pin 16 es alimentación; pin 1 es una salida. UDN pin 9 es alimentación y pin 10 es masa.** Montar con alimentación desconectada.

## Registro de cada dígito

| SN74HC595N | Conexión |
|---|---|
| 16 VCC | +5 V de lógica |
| 8 GND | Masa común |
| 10 /SRCLR | +5 V, nivel definido |
| 11 SRCLK | Mega 52, común a los cuatro registros |
| 12 RCLK | Mega 49, común a los cuatro registros |
| 13 /OE | Mega 48, común; una resistencia compartida de 10 kΩ a +5 V |
| 14 SER | Mega 51 en millares; pin 9 del registro anterior en los siguientes |
| 9 QH′ | Pin 14 del siguiente registro; libre en unidades |

Orden de la cadena: **Mega → millares → centenas → decenas → unidades**. El firmware envía unidades primero y millares al final; un único registro conectado representa millares. El latch aplica los cuatro patrones a la vez.

## Driver y conexiones de segmentos

UDN **9 → alimentación de segmentos** (5 V pequeño / 18 V grande), **10 → masa común**. Cada entrada UDN 1–8 lleva una resistencia de **10 kΩ a masa**, además de su conexión a la salida SN. No conectar esas entradas directamente a masa en el montaje completo. Retirar los puentes manuales de prueba a +5 V.

| Segmento | SN salida | UDN entrada | UDN salida | CL5611AH | ELS-50101AS |
|---|---:|---:|---:|---:|---:|
| A | 15 | 1 | 18 | 7 | 7 |
| B | 1 | 2 | 17 | 6 | 6 |
| C | 2 | 3 | 16 | 4 | 4 |
| D | 3 | 4 | 15 | 2 | 3 |
| E | 4 | 5 | 14 | 1 | 2 |
| F | 5 | 6 | 13 | 9 | 9 |
| G | 6 | 7 | 12 | 10 | 10 |
| DP | 7 | 8 | 11 | 5 | 5 |
| Cátodos a GND | — | — | — | **3 y 8** | **1 y 8** |

Cada salida UDN llega a su ánodo **a través de una resistencia independiente** según la variante. No colocar una sola resistencia en los cátodos comunes. Verificar ausencia de puentes entre salidas y que ninguna resistencia queda puenteada por la tira interna del protoboard.

## Condensadores y lista de materiales

Todos los condensadores van en paralelo entre su alimentación y masa, nunca en serie con los segmentos. Los cerámicos no tienen polaridad; en electrolíticos, positivo al rail positivo y negativo marcado con franja a masa.

| Material | Un dígito | Cuatro dígitos |
|---|---:|---:|
| Visualizador de la variante elegida | 1 | 4 |
| SN74HC595N DIP16 | 1 | 4 |
| UDN2981A DIP18 | 1 | 4 |
| Resistencias de segmentos A–G | 7 | 28 |
| Resistencia DP | 1 | 4 |
| Pull-down 10 kΩ, entradas UDN | 8 | 32 |
| Pull-up 10 kΩ, /OE compartido | 1 | 1 |
| Cerámico 100 nF, 50 V | 2 | 8 |
| Electrolítico 100 µF, 35 V o superior | 1 con rail único de 5 V | 1 por entrada de rail: 5 V lógica y alimentación de segmentos |

100 nF junto a cada SN entre **16 y 8**, y junto a cada UDN entre **9 y 10**, con conexiones cortas. Reserva de 100 µF en la entrada de cada rail. Para cuatro pequeños alimentados desde el mismo rail basta una entrada común; la distribución y longitud del cableado pueden requerir reserva adicional. Las resistencias de 1 W son válidas también en la variante pequeña.

## Alimentación de la variante grande

Fuente de 12 V → elevador **Pololu U3V70A** → rail de 18 V exclusivo de los UDN. Ajustar con multímetro **antes de conectar visualizadores**. El Mega conserva su alimentación de barril de 12 V y los SN conservan 5 V. No aplicar 18 V a la lógica ni a los pequeños. Unir las masas; no unir los positivos de 5, 12 y 18 V.

El presupuesto de alimentación para cuatro dígitos a 20 mA por cada uno de ocho segmentos es 0,64 A a 18 V (11,52 W); ese presupuesto no obliga a operar los LED a 20 mA. Reservar al menos 2 A de capacidad en la rama de 12 V del elevador y comprobar regulación y temperatura bajo carga. El UDN también debe cumplir el límite térmico total de sus ocho canales.

## Puesta en servicio

1. Comprobar tensión, polaridad, valor de cada resistencia y correspondencia de pines sin alimentación. Usar una fuente con limitación de corriente para la primera energización.
2. Comprobar un segmento con su resistencia antes de conectar el registro. Después comprobar registro y driver con carga limitada; no conectar una salida directamente a +5 V o GND.
3. En cada canal de dígito, 0 apaga; 21–27 seleccionan A–G; **87 selecciona DP solo**. Comprobar uno por uno antes de activar todos.
4. **18 = 8.**, todos los segmentos; 1–9 = cifras sin punto, 10 = 0, 11–19 = 1.–9., 20 = 0. Ver [mapa DMX](MAPA_DE_CANALES_DMX.md).
5. Al reiniciar se muestran todos los segmentos dos veces. QLC+ debe tratar los dígitos como Effect/LTP, sin escalarlos con el maestro de intensidad.

Si aparece calentamiento, olor o humo, desconectar alimentación y sustituir cualquier componente dañado antes de continuar. No reutilizar un visualizador que haya emitido humo.

## Fichas técnicas

- [CL5611AH — XLITX](datasheets/CL5611AH-XLITX.pdf).
- [ELS-50101A/B — Etnal](datasheets/ELS-50101A-B-Etnal.pdf): referencia para la variante grande.
- [UDN2981A — Allegro](datasheets/UDN2981A-Allegro.pdf).
- [SN74HC595 — TI](https://www.ti.com/lit/ds/symlink/sn74hc595.pdf).
