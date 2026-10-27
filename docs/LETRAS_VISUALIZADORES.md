# Letras de los visualizadores — Control v0.0.9

Cada uno de los cuatro canales de dígito puede seleccionar letras. La ocupación sigue siendo de 19 canales y se conservan los valores 0–34 y los modos LED de v0.0.7.

- **35–60:** A–Z, en orden alfabético.
- **61–86:** a–z, en orden alfabético.
- **87–255:** sin asignar; visualizador apagado.

Las letras no encienden el punto decimal. Siete segmentos no pueden representar todas las letras fielmente: B y D usan formas parecidas a b y d; K, M, N, R, V, W y X son especialmente aproximadas. Algunas mayúsculas, minúsculas y cifras comparten patrón. El código DMX distingue las 52 letras aunque el glifo visible pueda coincidir. Para texto inequívoco se necesitarían visualizadores de más segmentos o una matriz.

## Mapa de valores y segmentos

Los nombres A–G de segmento siguen el dibujo; el punto decimal es independiente.

```text
  — A —
 F     B
  — G —
 E     C
  — D —  DP
```

| Mayúscula | DMX | Segmentos | Minúscula | DMX | Segmentos |
|---|---:|---|---|---:|---|
| A | 35 | ABCEFG | a | 61 | ABCDEG |
| B | 36 | CDEFG | b | 62 | CDEFG |
| C | 37 | ADEF | c | 63 | DEG |
| D | 38 | BCDEG | d | 64 | BCDEG |
| E | 39 | ADEFG | e | 65 | ABDEFG |
| F | 40 | AEFG | f | 66 | AEFG |
| G | 41 | ACDEF | g | 67 | ABCDFG |
| H | 42 | BCEFG | h | 68 | CEFG |
| I | 43 | BC | i | 69 | C |
| J | 44 | BCDE | j | 70 | BCD |
| K | 45 | ACEFG | k | 71 | ACEFG |
| L | 46 | DEF | l | 72 | EF |
| M | 47 | ABCEF | m | 73 | ACEG |
| N | 48 | CEG | n | 74 | CEG |
| O | 49 | ABCDEF | o | 75 | CDEG |
| P | 50 | ABEFG | p | 76 | ABEFG |
| Q | 51 | ABCFG | q | 77 | ABCFG |
| R | 52 | EG | r | 78 | EG |
| S | 53 | ACDFG | s | 79 | ACDFG |
| T | 54 | DEFG | t | 80 | DEFG |
| U | 55 | BCDEF | u | 81 | CDE |
| V | 56 | CDE | v | 82 | CDE |
| W | 57 | BDF | w | 83 | BDF |
| X | 58 | BCEFG | x | 84 | BCEFG |
| Y | 59 | BCDFG | y | 85 | BCDFG |
| Z | 60 | ABDEG | z | 86 | ABDEG |

Por ejemplo, para mostrar `AbCd`, enviar 35, 62, 37 y 64 a los cuatro canales de dígito. El valor 0 sigue apagando todos los segmentos.

La tabla de máscaras se implementa en `arduino/controlador/digitos.h`, con bits 0–6 para A–G y bit 7 para DP. La simulación comprueba la codificación y el envío a las cuatro salidas; la lectura visual y el circuito de los visualizadores de cátodo común siguen pendientes de validación física.
