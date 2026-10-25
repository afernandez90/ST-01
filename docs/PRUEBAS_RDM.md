# Prueba física de RDM

Fecha: 25 de octubre de 2026. Mega2560, adaptador DollaTek B07DJ4TGY3, RX1 en 19 y TX1 en 18; controlador de pruebas con interfaz DMX/RDM. Sketch cargado por USB con el cargador original.

Identidad final comprobada: **7FF0:66601337**. El campo de dispositivo se interpreta en hexadecimal, según pidió el usuario. Firmware y personalidad reportados: **0.0.5**; modo de QLC+ en repositorio, espacio de trabajo e instalación local: **v0.0.5**. Ocupación: 19 canales.

| Comprobación | Resultado |
|---|---|
| Descubrimiento y suma codificada de respuesta | UID esperado recibido |
| DISC_MUTE / DISC_UN_MUTE | Silencio y recuperación de descubrimiento confirmados |
| DEVICE_INFO | Modelo 1, firmware numérico 5, 19 canales y dirección base coherente |
| Modelo / fabricante / firmware / personalidad | Lecturas correctas: ST-01, Salto Temporal, 0.0.5 |
| IDENTIFY_DEVICE | GET/SET de activación/desactivación confirmado |
| DMX_START_ADDRESS | Cambio temporal de 1 a 2 y lectura de 2 correctos |
| DEVICE_LABEL | Cambio y lectura de etiqueta temporal correctos |
| Persistencia | Reinicio por DTR confirmado mediante mensaje [INICIO]; dirección y etiqueta conservadas |
| Límites de dirección | 0 y 495 rechazados con NACK DATA_OUT_OF_RANGE |
| Restauración | Dirección 1, etiqueta ST-01 e identificación desactivada confirmadas |
| FTC con cargador original | No se anuncia, como corresponde |
| Regresión DMX | LNB a 128 y después 0; recepción de unos 39–40 paquetes/s; apagado por pérdida de señal confirmado al detener el envío |

La comprobación de IDENTIFY valida el intercambio RDM y el estado que acepta el firmware. No verifica visualmente todas las luces: todavía hay que completar/probar los pilotos, tiras y controladores de números del montaje elegido. La medición eléctrica de los tiempos, las pruebas con otras interfaces y la actualización real mediante FTC siguen pendientes.

## Corrección del controlador de pruebas

Los primeros intentos descubrían el dispositivo, pero no recibían respuestas normales. La interfaz de pruebas filtraba las respuestas según su UID propio y la herramienta utilizaba un UID genérico. Se corrigió el transporte de la herramienta para consultar la identidad de la interfaz en cada conexión. Este requisito pertenece al protocolo serie de esa interfaz; no al firmware de ST-01.

El Arduino nunca utiliza ese UID como destino fijo: copia el UID fuente de cada petición al UID destino de su respuesta y utiliza su propia identidad como fuente. La identidad ST-01 del respondedor y del cargador se define en `identidad.h`.

Se probó una alternativa de break por GPIO durante el diagnóstico. La causa era el UID del controlador; se ha conservado la generación UART original (cero a 50 kbaud, 180 µs de break y 40 µs de marca), comprobada también con las respuestas normales después de corregir la herramienta.

Los diagnósticos USB añadidos muestran bytes recibidos, breaks, inicios/tramas RDM, respuestas iniciadas y terminadas, fase TX y errores UART cuando hay actividad RDM o recepción anómala. Se calculan por ventana, fuera de las interrupciones. El informe periódico LNB/paquetes por segundo se conserva.

## Repetir la prueba

Cerrar las aplicaciones que utilicen la interfaz para liberar el puerto. Conectar el Mega por USB y el enlace RS-485 bidireccional. Ejecutar:

```sh
python3 herramientas/probar_rdm.py --puerto PUERTO_RDM --completo --mega PUERTO_MEGA
```

Sustituya `PUERTO_RDM` y `PUERTO_MEGA` por los puertos detectados en su equipo. La utilidad requiere una interfaz compatible con el protocolo serie que implementa su transporte; para otros controladores, use sus herramientas RDM.

`--completo` habilita los cambios temporales y su restauración; `--mega` permite verificar persistencia tras un reinicio USB. El guion usa la identidad ST-01 configurada por defecto; puede cambiarse con `--uid`. Sin `--completo` solo se leen parámetros y se comprueba descubrimiento/silencio.

Esta prueba se realizó mediante la herramienta del proyecto y una interfaz DMX/RDM, no mediante una interfaz de gestión RDM de QLC+.
