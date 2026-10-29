# Mapa de canales DMX de ST-01

**Personalidad:** `Control v1.0.0`
**Ocupación:** 19 canales DMX
**Direccionamiento:** empieza en uno en este documento y en el firmware.

El modo `.qxf` de QLC+ utiliza atributos `Number` que empiezan en cero. El firmware recibe canales DMX convencionales, por lo que debe utilizar los desplazamientos que empiezan en uno de esta tabla.

| Desplazamiento | Nombre en QLC+ | Valores |
|---:|---|---|
| 1 | Millares | 0–87 |
| 2 | Centenas | 0–87 |
| 3 | Decenas | 0–87 |
| 4 | Unidades | 0–87 |
| 5 | Barra LED R | 0–255 |
| 6 | Barra LED G | 0–255 |
| 7 | Barra LED B | 0–255 |
| 8 | Modo Barra LED | 0–255: [efectos](EFECTOS_LED.md) |
| 9 | LED Azul | 0–255 |
| 10 | LED Verde | 0–255 |
| 11 | LED Naranja (A) | 0–255 |
| 12 | LED Rojo | 0–255 |
| 13 | LED Naranja (B) | 0–255 |
| 14 | LED Blanco Izquierdo | 0–255 |
| 15 | LED Blanco Derecho | 0–255 |
| 16 | Tira LED R | 0–255 |
| 17 | Tira LED G | 0–255 |
| 18 | Tira LED B | 0–255 |
| 19 | Modo Tira LED | 0–255: [efectos](EFECTOS_LED.md) |

## Codificación de los visualizadores

| Valor | Salida |
|---:|---|
| 0 | Apaga todos los segmentos, incluido el punto decimal |
| 1–9 | Muestra el dígito indicado (1–9) |
| 10 | Muestra el dígito 0 |
| 11–19 | Muestra el dígito `valor - 10` con punto decimal (1–9) |
| 20 | Muestra 0 con punto decimal |
| 21–27 | Muestra el segmento individual A–G |
| 28–34 | Muestra el segmento individual A–G con punto decimal |
| 35–60 | Letras A–Z: `35 + posición de la letra` (A = 0) |
| 61–86 | Letras a–z: `61 + posición de la letra` (a = 0) |
| 87 | Solo punto decimal |
| 88–255 | Sin asignar; apaga el visualizador |

Para una dirección DMX base `direccionBase`, lea el primer canal en `direccionBase` y el último en `direccionBase + 18`.

Las letras son aproximaciones de siete segmentos: varias mayúsculas se representan con formas minúsculas y algunos caracteres comparten glifo. Los valores 35–86 no encienden el punto decimal. Consulte el [mapa de letras y segmentos](LETRAS_VISUALIZADORES.md).
