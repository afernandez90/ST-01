# Efectos LED — Control v0.0.6

Los desplazamientos 8 (Barra) y 19 (Tira) eligen el efecto y su velocidad. Los canales RGB conservan el color y la intensidad máxima: ningún efecto añade blanco ni cambia el tono. RGB = 0 apaga cualquier efecto. La ocupación sigue siendo de 19 canales; dirección base 1–494.

| Efecto | Barra | Tira |
|---|---:|---:|
| Luz fija | 0 | 0 |
| Progreso manual | 1–60 | — |
| Respiración suave | 61–68 | 1–10 |
| Latido doble | 69–76 | 11–21 |
| Parpadeo regular | 77–84 | 22–31 |
| Estrobo | 85–92 | 32–42 |
| Luz corredora hacia delante | 93–100 | 43–53 |
| Luz corredora hacia atrás | 101–108 | 54–63 |
| Cometa hacia delante | 109–116 | 64–74 |
| Cometa hacia atrás | 117–125 | 75–85 |
| Barrido de ida y vuelta | 126–133 | 86–95 |
| Llenado y vaciado progresivo | 134–141 | 96–106 |
| Expansión desde el centro | 142–149 | 107–116 |
| Contracción hacia el centro | 150–157 | 117–127 |
| Dos luces que se cruzan | 158–165 | 128–138 |
| Tren de pulsos hacia delante | 166–173 | 139–148 |
| Tren de pulsos hacia atrás | 174–181 | 149–159 |
| Ondas de brillo | 182–190 | 160–170 |
| Chispas aleatorias | 191–198 | 171–180 |
| Parpadeo tipo llama | 199–206 | 181–191 |
| Relámpagos | 207–214 | 192–201 |
| Fallo eléctrico | 215–222 | 202–212 |
| Acumulación y descarga | 223–230 | 213–223 |
| Explosión desde el centro | 231–238 | 224–233 |
| Alternancia de segmentos | 239–246 | 234–244 |
| Aparición y desvanecimiento | 247–255 | 245–255 |

## Progreso manual y barrido

La Barra tiene provisionalmente 60 píxeles direccionables. Modo 1 enciende el primero; modo 2, los dos primeros; …; modo 60, los 60. Cada paso añade exactamente un píxel desde el extremo de entrada de datos. Para 0 píxeles, enviar RGB = 0. El progreso no avanza por tiempo: lo controla la sesión DMX. Se reservan 60 submodos; si cambia el recuento real, hay que revisar esta asignación, el fixture y la comprobación de compilación. Con más de 231 píxeles no cabe un paso por píxel junto a las 24 animaciones en un único canal de 8 bits.

El barrido de ida y vuelta utiliza un píxel al 100 % y sus vecinos inmediatos al 60 %, cuando existen. Rebota en ambos extremos sin envolver los vecinos al otro extremo. El color depende del RGB; para el efecto KITT, seleccionar rojo. El ciclo es una ida y una vuelta completas.

## Velocidades y tiempos

Dentro de cada clase, el mínimo es la velocidad más lenta y el máximo la más rápida. Los valores intermedios interpolan linealmente la duración: `6000 − (5000 × paso / (pasos − 1))` ms, con división entera. Cada clase tiene 8–9 velocidades en la Barra y 10–11 en la Tira. El estrobo usa `500 − (400 × paso / (pasos − 1))` ms (2–10 Hz). Parpadeo y estrobo tienen ciclo de trabajo del 50 %.

El modo se reinicia al cambiar su valor; cambiar solo RGB conserva la fase. Cada tira mantiene su propio reloj. El inicio es el extremo de entrada de datos, no una orientación física ya verificada. Los efectos se calculan sin bloqueos ni memoria adicional por píxel. El refresco sigue siendo 20 Hz; la velocidad aparente depende del número real de píxeles y de la transmisión.

## Comportamiento de las animaciones

Respiración usa una curva suave de subida y bajada. Latido doble produce dos pulsos, el segundo más tenue, seguidos de oscuridad. Las corredoras usan un tramo de aproximadamente el 10 %; los cometas añaden una cola decreciente de aproximadamente el 30 %. El llenado automático recorre de vacío a lleno y vuelve a vacío. Expansión y contracción desplazan frentes simétricos; dos luces que se cruzan recorre ambos sentidos a la vez. Los trenes alternan tramos encendidos y huecos; las ondas modulan suavemente el brillo por posición.

Chispas produce puntos breves con caída de brillo; llama interpola intensidades por zonas; relámpagos alterna ráfagas y ciclos oscuros; fallo eléctrico interrumpe zonas de forma irregular. Acumulación acelera los pulsos y termina con toda la tira encendida. Explosión emite un frente desde el centro con desvanecimiento posterior. Alternancia intercambia zonas pares e impares. Aparición y desvanecimiento usa zonas con fases pseudoaleatorias. La aleatoriedad es determinista para que las pruebas sean reproducibles.

## Compatibilidad y validación

v0.0.6 conserva modo 0, RGB y todos los desplazamientos de v0.0.5. Los antiguos valores de modo distintos de 0 ahora activan efectos. QLC+ debe usar el fixture y el espacio de trabajo actualizados; los selectores de modo no deben escalarse con el maestro. Identificar fuerza RGB fijo blanco intermitente y la pérdida de DMX apaga todas las salidas, también durante una animación.

Las pruebas automatizadas cubren asignación, progreso, barrido, límites y simulación de salidas. Falta validar físicamente orientación, número de píxeles, fluidez y convivencia con DMX/RDM. No se ha cargado el firmware ni instalado el fixture fuera del repositorio.
