# Mapa de canales DMX de ST-01

**Personalidad:** `Control v0.0.1`
**Ocupación:** 20 canales DMX
**Direccionamiento:** empieza en uno en este documento y en el firmware.

El modo `.qxf` de QLC+ utiliza atributos `Number` que empiezan en cero. El firmware recibe canales DMX convencionales, por lo que debe utilizar los desplazamientos que empiezan en uno de esta tabla.

| Desplazamiento | Nombre en QLC+ | Símbolo en código | Valores |
|---:|---|---|---|
| 1 | Master Dimmer | `CH_MASTER_DIMMER` | 0–255 |
| 2 | Millares | `CH_DIGIT_THOUSANDS` | 0–34 |
| 3 | Centenas | `CH_DIGIT_HUNDREDS` | 0–34 |
| 4 | Decenas | `CH_DIGIT_TENS` | 0–34 |
| 5 | Unidades | `CH_DIGIT_UNITS` | 0–34 |
| 6 | Barra LED R | `CH_BAR_R` | 0–255 |
| 7 | Barra LED G | `CH_BAR_G` | 0–255 |
| 8 | Barra LED B | `CH_BAR_B` | 0–255 |
| 9 | Modo Barra LED | `CH_BAR_MODE` | 0: RGB fijo |
| 10 | LED Azul | `CH_LAMP_BLUE` | 0–255 |
| 11 | LED Verde | `CH_LAMP_GREEN` | 0–255 |
| 12 | LED Naranja (A) | `CH_LAMP_ORANGE_A` | 0–255 |
| 13 | LED Rojo | `CH_LAMP_RED` | 0–255 |
| 14 | LED Naranja (B) | `CH_LAMP_ORANGE_B` | 0–255 |
| 15 | LED Blanco Izquierdo | `CH_LAMP_WHITE_LEFT` | 0–255 |
| 16 | LED Blanco Derecho | `CH_LAMP_WHITE_RIGHT` | 0–255 |
| 17 | Tira LED R | `CH_STRIP_R` | 0–255 |
| 18 | Tira LED G | `CH_STRIP_G` | 0–255 |
| 19 | Tira LED B | `CH_STRIP_B` | 0–255 |
| 20 | Modo Tira LED | `CH_STRIP_MODE` | 0: RGB fijo |

## Codificación de los visualizadores

| Valor | Salida |
|---:|---|
| 0–9 | Dígito decimal 0–9 |
| 10–19 | Dígito decimal 0–9 con punto decimal |
| 20–26 | Segmentos A, B, C, D, E, F, G respectivamente |
| 27–33 | Segmentos A, B, C, D, E, F, G respectivamente, con punto decimal |
| 34 | Apaga todos los segmentos, incluido el punto decimal |

Para una dirección DMX base `direccionBase`, lea el primer canal en `direccionBase` y el último en `direccionBase + 19`.
