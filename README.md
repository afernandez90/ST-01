# Controlador DMX ST-01

ST-01 es un controlador DMX512 personalizado basado en Arduino Mega 2560, con gestión remota mediante RDM. Recibe las señales independientemente de la marca o modelo del emisor DMX.

## Personalidad DMX: Control v0.0.6

ST-01 ocupa 19 canales consecutivos. La tabla usa desplazamientos que empiezan en uno; los índices internos del fixture de QLC+ empiezan en cero.

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

Si la dirección base es **B**, los canales utilizados van de **B** a **B + 18**. La dirección base válida es 1–494. El dispositivo no tiene un canal maestro; el regulador del controlador debe actuar sobre las intensidades RGB y de pilotos, sin escalar los selectores de dígitos ni de modo.

## Documentación de usuario

- [Manual de usuario](docs/MANUAL_DE_USUARIO.md): conexión, dirección DMX, controles locales y uso con QLC+.
- [Mapa de canales](docs/MAPA_DE_CANALES_DMX.md): valores y codificación de cada función.
- [Efectos LED](docs/EFECTOS_LED.md): clases, rangos de modo, velocidades y progreso manual.
- [Gestión RDM](docs/RDM.md): identificación, dirección, etiqueta y parámetros remotos.
- [Actualización por RDM](docs/ACTUALIZACION_RDM.md): requisitos, procedimiento y límites de la actualización de firmware.

## Guía del repositorio para desarrollo

| Ruta | Contenido |
|---|---|
| `arduino/pruebas/recepcion_dmx/` | Firmware activo del prototipo: recepción DMX, RDM, efectos, salidas y controles locales. |
| `arduino/pruebas/` | Pruebas de hardware, incluida recepción DMX, parpadeo y escáner I2C. |
| `arduino/arranque_rdm/` | Cargador FTC para Mega2560, compilación e instrucciones de instalación. |
| `arduino/controlador/` | Carpeta reservada para el firmware final tras validar el montaje. |
| `herramientas/` | Utilidades de prueba RDM, empaquetado, transferencia y verificación del cargador. |
| `pruebas/` | Pruebas automatizadas y simulación de Arduino para ejecutarlas sin placa. |
| `qlcplus/fixtures/ST-01.qxf` | Definición del fixture y sus capacidades DMX. |
| `qlcplus/SaltoTemporal.qxw` | Espacio de trabajo y Consola Virtual; configurar la salida del universo según el equipo disponible. |
| `qlcplus/models/` | Maqueta 3D provisional, materiales, vista previa y generador. |
| `docs/` | Documentación de usuario, diseño, validación y continuidad. |
| `ide/` | Configuraciones de desarrollo organizadas por plataforma, con instrucciones de uso en cada carpeta. |

Para preparar el entorno y compilar, consulte las [pruebas de hardware](arduino/pruebas/LEEME.md) y el [manual del cargador](arduino/arranque_rdm/LEEME.md). La suite sin placa se ejecuta desde la raíz con `sh pruebas/rdm.sh`; requiere un shell compatible, compilador C++ con sanitizadores y Python 3.

El [cableado propuesto](docs/CABLEADO_PROPUESTO.md), las [pruebas físicas RDM](docs/PRUEBAS_RDM.md), la [depuración RDM](docs/DEPURACION_RDM.md) y la [continuidad del proyecto](docs/CONTINUIDAD.md) recogen las decisiones y tareas pendientes. DMX y RDM básico se han probado físicamente; el montaje completo de salidas y la instalación inicial del cargador FTC siguen pendientes de validación.

Al cambiar la personalidad, mantener alineados firmware, fixture, espacio de trabajo y documentación. La versión del firmware se define en `arduino/pruebas/recepcion_dmx/version.h`.
