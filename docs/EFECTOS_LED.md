# Efectos LED — Control v1.0.0

Los desplazamientos 8 (Barra) y 19 (Tira) eligen el efecto y su velocidad. Los canales RGB conservan el color y la intensidad máxima: ningún efecto añade blanco ni cambia el tono. RGB = 0 apaga cualquier efecto. La ocupación sigue siendo de 19 canales; dirección base 1–494.

| Efecto | Barra | Tira |
|---|---:|---:|
| Luz fija | 0 | 0 |
| Progreso manual | 1–38 | — |
| Respiración suave | 39–47 | 1–10 |
| Latido doble | 48–56 | 11–21 |
| Parpadeo regular | 57–65 | 22–31 |
| Estrobo | 66–74 | 32–42 |
| Luz corredora hacia delante | 75–83 | 43–53 |
| Luz corredora hacia atrás | 84–92 | 54–63 |
| Cometa hacia delante | 93–101 | 64–74 |
| Cometa hacia atrás | 102–110 | 75–85 |
| Barrido de ida y vuelta | 111–119 | 86–95 |
| Llenado y vaciado progresivo | 120–128 | 96–106 |
| Expansión desde el centro | 129–137 | 107–116 |
| Contracción hacia el centro | 138–146 | 117–127 |
| Dos luces que se cruzan | 147–155 | 128–138 |
| Tren de pulsos hacia delante | 156–164 | 139–148 |
| Tren de pulsos hacia atrás | 165–173 | 149–159 |
| Ondas de brillo | 174–182 | 160–170 |
| Chispas aleatorias | 183–191 | 171–180 |
| Parpadeo tipo llama | 192–200 | 181–191 |
| Relámpagos | 201–209 | 192–201 |
| Fallo eléctrico | 210–218 | 202–212 |
| Acumulación y descarga | 219–227 | 213–223 |
| Explosión desde el centro | 228–236 | 224–233 |
| Alternancia de segmentos | 237–245 | 234–244 |
| Aparición y desvanecimiento | 246–255 | 245–255 |

## Progreso manual y barrido

La Barra tiene **38 píxeles direccionables**, comprobados físicamente el 26 de octubre de 2026: con el firmware anterior, el último se encendió al seleccionar modo 38. Modo 1 enciende el primero; modo 2, los dos primeros; …; modo 38, los 38. Cada paso añade exactamente un píxel desde el extremo de entrada de datos. Para 0 píxeles, enviar RGB = 0. El progreso no avanza por tiempo: lo controla la sesión DMX. Se reservan 38 submodos; si cambia el recuento real, hay que revisar esta asignación, el fixture y la comprobación de compilación. Con más de 231 píxeles no cabe un paso por píxel junto a las 24 animaciones en un único canal de 8 bits.

El barrido de ida y vuelta utiliza una cabeza al 100 % y diez píxeles por detrás al 10, 9, 8, 7, 6, 5, 4, 3, 2 y 1 %. Son once posiciones contando la cabeza. Todos los píxeles por delante permanecen apagados. Los porcentajes multiplican el RGB recibido, por lo que el maestro de intensidad sigue regulando el efecto. Al rebotar se invierte la dirección de la cola. En los extremos se recorta la parte que queda fuera de la tira, sin envolver al otro extremo. El color depende del RGB; para el efecto KITT, seleccionar rojo. El ciclo es una ida y una vuelta completas.

## Velocidades y tiempos

Los ciclos generales duran 3000–500 ms. El estrobo utiliza ciclos de 250–100 ms (4–10 Hz), con refresco de 50 ms.

Dentro de cada clase, el mínimo es la velocidad más lenta y el máximo la más rápida. Los valores intermedios interpolan linealmente la duración: `3000 − (2500 × paso / (pasos − 1))` ms, con división entera. Cada clase tiene 9–10 velocidades en la Barra y 10–11 en la Tira. El estrobo usa `250 − (150 × paso / (pasos − 1))` ms (4–10 Hz), con división entera. Parpadeo y estrobo tienen ciclo de trabajo del 50 %.

El modo se reinicia al cambiar su valor; cambiar solo RGB conserva la fase. Cada tira mantiene su propio reloj. El inicio corresponde al extremo de entrada de datos. El refresco sigue siendo 20 Hz; la velocidad aparente depende del número real de píxeles y de la transmisión.

## Comportamiento de las animaciones

Respiración usa una curva suave de subida y bajada. Latido doble produce dos pulsos, el segundo más tenue, seguidos de oscuridad. Las corredoras usan un tramo de aproximadamente el 10 %; los cometas añaden una cola decreciente de aproximadamente el 30 %. El llenado automático recorre de vacío a lleno y vuelve a vacío. Expansión y contracción desplazan frentes simétricos; dos luces que se cruzan recorre ambos sentidos a la vez. Los trenes alternan tramos encendidos y huecos; las ondas modulan suavemente el brillo por posición.

Chispas produce puntos breves con caída de brillo; llama interpola intensidades por zonas; relámpagos alterna ráfagas y ciclos oscuros; fallo eléctrico interrumpe zonas de forma irregular. Acumulación acelera los pulsos y termina con toda la tira encendida. Explosión emite un frente desde el centro con desvanecimiento posterior. Alternancia intercambia zonas pares e impares. Aparición y desvanecimiento usa zonas con fases pseudoaleatorias.

## Uso con la consola

Utilizar el fixture v1.0.0. Los canales RGB responden al maestro de intensidad; los selectores de modo no deben escalarse. Identificación fuerza blanco intermitente y la pérdida de DMX apaga las salidas. Los porcentajes de los efectos multiplican el RGB recibido.
