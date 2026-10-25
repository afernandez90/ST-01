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

El espacio de trabajo incluido está configurado para el Universo 1 mediante un ENTTEC DMX USB Pro. ST-01 ocupa 20 canales DMX consecutivos. La instancia suministrada empieza en la dirección DMX 1.

## Personalidad DMX: Control v0.0.1

La definición `.qxf` utiliza índices de canal internos que empiezan en cero. La siguiente tabla usa los desplazamientos DMX convencionales, que empiezan en uno.

| Desplazamiento | Función |
|---:|---|
| 1 | Regulador maestro |
| 2 | Millares |
| 3 | Centenas |
| 4 | Decenas |
| 5 | Unidades |
| 6–8 | Barra LED: rojo, verde y azul |
| 9 | Modo de Barra LED |
| 10 | Piloto azul |
| 11 | Piloto verde |
| 12 | Piloto naranja A |
| 13 | Piloto rojo |
| 14 | Piloto naranja B |
| 15 | Piloto blanco izquierdo |
| 16 | Piloto blanco derecho |
| 17–19 | Tira LED: rojo, verde y azul |
| 20 | Modo de Tira LED |

Si la dirección base del fixture es **B**, el desplazamiento 1 corresponde al canal DMX **B** y el desplazamiento 20 corresponde al canal **B + 19**.

Los canales de visualizador usan estos valores:

| Valor | Resultado |
|---:|---|
| 0–9 | Muestra ese dígito |
| 10–19 | Muestra el dígito `valor - 10` con punto decimal |
| 20–26 | Muestra el segmento individual A–G |
| 27–33 | Muestra el segmento individual A–G con punto decimal |
| 34 | Apaga todos los segmentos, incluido el punto decimal |

En v0.0.1, el valor de modo `0` significa color RGB fijo. Se asignarán modos adicionales solo después de que el firmware de Arduino los implemente y se prueben.

## Estado de desarrollo

La personalidad de QLC+ y la Consola Virtual están definidas. Quedan por implementar y probar el firmware de Arduino, el cableado del receptor DMX, el recuento de píxeles de las tiras, el sistema de alimentación y los circuitos de salida.

Consulte la [guía de usuario](docs/MANUAL_DE_USUARIO.md) y el [mapa de canales para firmware](docs/MAPA_DE_CANALES_DMX.md).

Las próximas pruebas y el cambio futuro a MacBook están anotados en [continuidad del proyecto](docs/CONTINUIDAD.md).
