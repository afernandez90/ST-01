# Manual de usuario — ST-01 DMX Controller

## 1. Finalidad

ST-01 es un dispositivo DMX512 que controla cuatro dígitos numéricos, siete pilotos luminosos y dos tiras LED RGB direccionables. El dispositivo se integra en una línea DMX convencional y ocupa 20 canales consecutivos.

Esta edición del manual corresponde a la personalidad **Control v0.0.1**.

## 2. Conexión

1. Conecte una salida DMX del controlador al conector **DMX IN** de ST-01.
2. Si la línea continúa hacia otros equipos, conecte el siguiente cable al conector **DMX THRU**.
3. Encienda ST-01 mediante su interruptor principal.
4. Configure la dirección base DMX del equipo. La dirección debe dejar libres 20 canales consecutivos.

El último aparato de una cadena DMX debe terminarse con un terminador DMX de 120 Ω.

## 3. Dirección DMX

Si la dirección base es **B**, ST-01 utiliza los canales desde **B** hasta **B + 19**. Por ejemplo, con dirección base 1 utiliza los canales 1–20; con dirección base 101 utiliza los canales 101–120.

## 4. Canales DMX

| Desplazamiento | Función |
|---:|---|
| 1 | Regulador maestro |
| 2–5 | Millares, centenas, decenas y unidades |
| 6–8 | Barra LED: rojo, verde y azul |
| 9 | Modo de la Barra LED |
| 10–16 | Pilotos: azul, verde, naranja A, rojo, naranja B, blanco izquierdo y blanco derecho |
| 17–19 | Tira LED: rojo, verde y azul |
| 20 | Modo de la Tira LED |

Los controles RGB aceptan valores de 0 a 255. El valor 0 apaga ese componente y 255 aplica la intensidad máxima.

En la versión actual, el valor 0 de cada canal de modo selecciona **Color fijo**. Los modos de animación se documentarán cuando estén disponibles en el firmware.

## 5. Control de los dígitos

Cada uno de los cuatro canales de dígito admite los siguientes valores:

| Valores DMX | Resultado |
|---:|---|
| 0–9 | Muestra el número indicado |
| 10–19 | Muestra el número indicado con punto decimal |
| 20–26 | Muestra únicamente los segmentos A–G |
| 27–33 | Muestra los segmentos A–G con punto decimal |

El orden de los segmentos es: A, B, C, D, E, F y G.

## 6. Proyecto QLC+

El paquete del proyecto incluye:

```text
qlcplus/fixtures/ST-01.qxf
qlcplus/SaltoTemporal.qxw
```

Para utilizarlo en Linux, copie `ST-01.qxf` en `~/.qlcplus/fixtures/`, reinicie QLC+ y abra `SaltoTemporal.qxw`.

El proyecto suministrado usa la salida **DMX USB** con una interfaz ENTTEC DMX USB Pro y sitúa ST-01 en el Universo 1, dirección 1. Cambie la dirección de la instancia desde el Gestor de Fixtures si el diseño DMX requiere otra dirección.

La Consola Virtual tiene dos páginas:

- **Cara A:** controles de los cuatro dígitos, Barra LED y cuatro pilotos.
- **Cara B:** tres pilotos, Tira LED y regulador maestro.

Ponga QLC+ en modo **Operate** para enviar los valores DMX.

## 7. Resolución de problemas

| Síntoma | Comprobación |
|---|---|
| ST-01 no responde | Compruebe alimentación, dirección base, cable DMX y que el controlador transmite en el universo correcto. |
| Los controles no producen salida | En QLC+, confirme que la salida DMX USB está marcada en Universo 1 y que el proyecto está en modo Operate. |
| Respuesta de un canal equivocado | Compare la dirección base configurada en ST-01 con la dirección de la instancia ST-01 en el software. |
| El último equipo falla o parpadea | Instale un terminador DMX en el último equipo de la línea. |

## 8. Límites de esta versión

Este manual describe la personalidad DMX ya definida en el proyecto. Las funciones físicas del controlador dependen de la versión de firmware instalada. Consulte la documentación de la versión de firmware entregada con el equipo para conocer las funciones de animación disponibles.
