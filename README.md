# Controlador DMX ST-01

ST-01 es un dispositivo DMX512 personalizado, basado en un Arduino Mega 2560. Controla cuatro visualizadores numéricos grandes, pilotos de panel y dos tiras LED RGB direccionables.

## Contenido del repositorio

```text
arduino/controlador/     Firmware para Arduino (se añadirá durante el prototipado)
docs/                    Personalidad DMX y documentación de usuario
qlcplus/                 Definición de fixture y espacio de trabajo de QLC+
```

## Archivos de QLC+

- `qlcplus/fixtures/ST-01.qxf` es la definición del fixture ST-01.
- `qlcplus/SaltoTemporal.qxw` es el espacio de trabajo de QLC+, incluida la Consola Virtual.

Para usar la definición del fixture en Linux, copie `ST-01.qxf` en `~/.qlcplus/fixtures/`, reinicie QLC+ y añada **Salto Temporal / ST-01** a un universo.

El espacio de trabajo incluido está configurado para el Universo 1 mediante un ENTTEC DMX USB Pro. ST-01 ocupa 19 canales DMX consecutivos. La instancia suministrada empieza en la dirección DMX 1.

## Personalidad DMX: Control v0.0.6

La definición `.qxf` utiliza índices de canal internos que empiezan en cero. La siguiente tabla usa los desplazamientos DMX convencionales, que empiezan en uno.

| Desplazamiento | Función |
|---:|---|
| 1 | Millares |
| 2 | Centenas |
| 3 | Decenas |
| 4 | Unidades |
| 5–7 | Barra LED: rojo, verde y azul |
| 8 | Modo de Barra LED |
| 9 | Piloto azul |
| 10 | Piloto verde |
| 11 | Piloto naranja A |
| 12 | Piloto rojo |
| 13 | Piloto naranja B |
| 14 | Piloto blanco izquierdo |
| 15 | Piloto blanco derecho |
| 16–18 | Tira LED: rojo, verde y azul |
| 19 | Modo de Tira LED |

Si la dirección base del fixture es **B**, el desplazamiento 1 corresponde al canal DMX **B** y el desplazamiento 19 corresponde al canal **B + 18**.

Los canales de visualizador usan estos valores:

| Valor | Resultado |
|---:|---|
| 0 | Apaga todos los segmentos, incluido el punto decimal |
| 1–9 | Muestra el dígito indicado (1–9) |
| 10 | Muestra el dígito 0 |
| 11–20 | Muestra el dígito `valor - 11` con punto decimal (0–9) |
| 21–27 | Muestra el segmento individual A–G |
| 28–34 | Muestra el segmento individual A–G con punto decimal |

El modo `0` conserva el color RGB fijo. Los canales de modo incluyen 24 animaciones con velocidades seleccionables; la Barra añade progreso manual de un píxel por valor. Consulte el [mapa de efectos](docs/EFECTOS_LED.md). Implementación probada en simulación; validación física pendiente.

El regulador maestro pertenece a QLC+ y no ocupa un canal del dispositivo. Debe actuar solo sobre las intensidades de RGB y pilotos; los dígitos se apagan con su valor 0.

## Estado de desarrollo

La personalidad de QLC+ y la Consola Virtual están definidas. La recepción DMX básica ya se ha probado. Las nuevas salidas físicas, el retorno RDM, el recuento de píxeles y los circuitos de potencia siguen pendientes de montaje y validación.

La prueba `arduino/pruebas/recepcion_dmx` incluye un [respondedor RDM](docs/RDM.md), identificación conjunta de todas las salidas y [actualización FTC](docs/ACTUALIZACION_RDM.md) con cargador propio para Mega. Compilación y simulación verificadas, y [RDM básico probado físicamente](docs/PRUEBAS_RDM.md). El montaje completo de salidas y la instalación inicial del cargador por ISP siguen pendientes. Se ha definido el [cableado del prototipo](docs/CABLEADO_PROPUESTO.md).

Consulte la [guía de usuario](docs/MANUAL_DE_USUARIO.md) y el [mapa de canales para firmware](docs/MAPA_DE_CANALES_DMX.md).

Las próximas pruebas y el cambio futuro a MacBook están anotados en [continuidad del proyecto](docs/CONTINUIDAD.md).
