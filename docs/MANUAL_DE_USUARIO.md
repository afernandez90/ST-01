# Manual de usuario — Controlador DMX ST-01

## 1. Finalidad

ST-01 es un dispositivo DMX512 que controla cuatro dígitos numéricos, siete pilotos luminosos y dos tiras LED RGB direccionables. El dispositivo se integra en una línea DMX convencional y ocupa 19 canales consecutivos.

Esta edición del manual corresponde a la personalidad **Control v1.0.0**.

## 2. Conexión

1. Conecte una salida DMX del controlador al conector **DMX IN** de ST-01.
2. Si la línea continúa hacia otros equipos, conecte el siguiente cable al conector **DMX THRU**.
3. Encienda ST-01 mediante su interruptor principal.
4. Configure la dirección base DMX del equipo. La dirección debe dejar libres 19 canales consecutivos.

Al arrancar, las luces y los dígitos realizan dos destellos de autocomprobación. Sin señal válida durante un segundo, las salidas se apagan.

El último aparato de una cadena DMX debe terminarse con un terminador DMX de 120 Ω.

## 3. Dirección DMX

Si la dirección base es **B**, ST-01 utiliza los canales desde **B** hasta **B + 18**. Por ejemplo, con dirección base 1 utiliza los canales 1–19; con dirección base 101 utiliza los canales 101–119.

## 4. Canales DMX

| Desplazamiento | Función |
|---:|---|
| 1–4 | Millares, centenas, decenas y unidades |
| 5–7 | Barra LED: rojo, verde y azul |
| 8 | Modo de la Barra LED |
| 9–15 | Pilotos: azul, verde, naranja A, rojo, naranja B, blanco izquierdo y blanco derecho |
| 16–18 | Tira LED: rojo, verde y azul |
| 19 | Modo de la Tira LED |

Los controles RGB aceptan valores de 0 a 255. El valor 0 apaga ese componente y 255 aplica la intensidad máxima.

El valor 0 selecciona **Luz fija**. Ambos canales ofrecen 24 animaciones con velocidades seleccionables; la Barra también permite progreso manual de 1 a 38 píxeles. Consulte [rangos, velocidades y comportamiento de los efectos](EFECTOS_LED.md).

El dispositivo no tiene un canal maestro. El maestro de QLC+ debe configurarse para reducir o limitar únicamente los canales de intensidad (RGB y pilotos). Los canales de dígitos y modos son selectores y no deben escalarse. Un blackout de intensidad apaga RGB y pilotos, pero mantiene los dígitos; para apagar también los dígitos, use blackout de todos los canales, que envía 0 a sus selectores.

## 5. Control de los dígitos

Cada uno de los cuatro canales de dígito admite los siguientes valores:

| Valores DMX | Resultado |
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

El orden de los segmentos es: A, B, C, D, E, F y G.

## 6. Proyecto QLC+

El paquete del proyecto incluye:

```text
qlcplus/fixtures/ST-01.qxf
qlcplus/SaltoTemporal.qxw
```

Importe `ST-01.qxf` en la biblioteca de fixtures de su instalación de QLC+, reinicie QLC+ y abra `SaltoTemporal.qxw`.

La instancia debe utilizar **Salto Temporal / ST-01 / v1.0.0**, no **Generic / 19 Channel**. La definición asigna los canales 1–4 (dígitos), 8 (modo Barra) y 19 (modo Tira) al grupo **Effect/LTP**, fuera del maestro de intensidad. Solo RGB y pilotos pertenecen a Intensity. Mantenga el Grand Master configurado para canales de intensidad; si se elige explícitamente actuar sobre todos los canales, también escalará dígitos y modos. Si aparece el problema tras actualizar, reinstale la definición y vuelva a abrir el workspace.

El proyecto sitúa ST-01 en el Universo 1, dirección 1. Configure la salida de ese universo para la interfaz o red DMX que utilice. El firmware recibe DMX512 sin depender de la marca o modelo del emisor. Cambie la dirección de la instancia desde el gestor de equipos si el diseño DMX requiere otra dirección.

La Consola Virtual tiene dos páginas:

- **Cara A:** controles de los cuatro dígitos, Barra LED y cuatro pilotos.
- **Cara B:** tres pilotos, Tira LED y maestro de consola (sin canal DMX propio).

Active la salida DMX del universo y utilice la Consola Virtual para enviar los valores.

## 7. Resolución de problemas

| Síntoma | Comprobación |
|---|---|
| ST-01 no responde | Compruebe alimentación, dirección base, cable DMX y que el controlador transmite en el universo correcto. |
| Los controles no producen salida | En QLC+, confirme la salida del Universo 1 y que el controlador esté conectado. |
| Respuesta de un canal equivocado | Compare la dirección base configurada en ST-01 con la dirección de la instancia ST-01 en el software. |
| El último equipo falla o parpadea | Instale un terminador DMX en el último equipo de la línea. |

## 8. Límites de esta versión

Utilice firmware, fixture y workspace de la misma versión v1.0.0. Consulte [cableado](CABLEADO.md) y [visualizadores](VISUALIZADORES.md) antes de montar o sustituir componentes.

Las letras son aproximaciones de siete segmentos: varias mayúsculas se representan con formas minúsculas y algunos caracteres comparten glifo. Los valores 35–86 no encienden el punto decimal. Consulte el [mapa de letras y segmentos](LETRAS_VISUALIZADORES.md).
