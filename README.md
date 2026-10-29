# Controlador DMX ST-01 — v1.0.0

ST-01 es un controlador DMX512 personalizado basado en Arduino Mega 2560, con gestión remota mediante RDM. Recibe las señales independientemente de la marca o modelo del emisor DMX.

## Personalidad DMX: Control v1.0.0

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
- [Cableado](docs/CABLEADO.md): conexiones del controlador, fuentes e interfaces.
- [Visualizadores](docs/VISUALIZADORES.md): circuito completo para CL5611AH y 50101AS.
- [Mapa de canales](docs/MAPA_DE_CANALES_DMX.md): valores y codificación de cada función.
- [Efectos LED](docs/EFECTOS_LED.md): clases, rangos de modo, velocidades y progreso manual.
- [Gestión RDM](docs/RDM.md): identificación, dirección, etiqueta y parámetros remotos.
- [Actualización por RDM](docs/ACTUALIZACION_RDM.md): requisitos, procedimiento y límites de la actualización de firmware.

## Guía del repositorio para desarrollo

| Ruta | Contenido |
|---|---|
| `arduino/controlador/` | Firmware principal: recepción DMX, RDM, efectos, salidas y controles locales. |
| `arduino/pruebas/` | Pruebas independientes de hardware: parpadeo y escáner I2C. |
| `arduino/arranque_rdm/` | Cargador FTC para Mega2560, compilación e instrucciones de instalación. |
| `herramientas/` | Utilidades de prueba RDM, empaquetado, transferencia y verificación del cargador. |
| `pruebas/` | Pruebas automatizadas y simulación de Arduino para ejecutarlas sin placa. |
| `qlcplus/fixtures/ST-01.qxf` | Definición del fixture y sus capacidades DMX. |
| `qlcplus/SaltoTemporal.qxw` | Espacio de trabajo y Consola Virtual; configurar la salida del universo según el equipo disponible. |
| `qlcplus/models/` | Modelo 3D, materiales, vista previa y generador. |
| `docs/` | Manuales, especificación de canales, cableado y fichas técnicas. |
| `ide/` | Configuraciones de desarrollo organizadas por plataforma, con instrucciones de uso en cada carpeta. |

Para preparar el entorno y compilar, consulte el [firmware principal](arduino/controlador/LEEME.md) y el [manual del cargador](arduino/arranque_rdm/LEEME.md). Las actualizaciones habituales del controlador se realizan por RDM con `sh herramientas/cargar_rdm.sh` o la tarea de VS Code **RDM: compilar y cargar controlador**. La suite sin placa se ejecuta desde la raíz con `sh pruebas/rdm.sh`; requiere un shell compatible, compilador C++ con sanitizadores y Python 3.

El [cableado](docs/CABLEADO.md) y la [guía de visualizadores](docs/VISUALIZADORES.md) describen las conexiones, componentes y puesta en servicio de ambas variantes.

Al cambiar la personalidad, mantener alineados firmware, fixture, espacio de trabajo y documentación. La versión del firmware se define en `arduino/controlador/version.h`.
