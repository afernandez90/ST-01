# Actualización de firmware — ST-01 v1.0.0

La vía habitual de actualización es **RDM/FTC 1.00**, con PIDs 0x1200–0x1203. El protocolo del Mega es independiente de la marca del emisor. La interfaz y el programa del ordenador deben transportar RDM y ejecutar la versión publicada de FTC; disponer de RDM básico no implica disponer de actualización FTC.

## Actualizar

1. Alimentar ST-01 y conectar su enlace DMX/RDM bidireccional.
2. Cerrar QLC+ y cualquier programa que ocupe la interfaz.
3. Ejecutar desde la raíz:

```sh
sh herramientas/cargar_rdm.sh
```

El lanzador compila el firmware actual, genera `build/controlador/ST-01.st01`, transfiere la imagen, valida CRC, confirma y consulta la versión tras reiniciar. No necesita USB del Mega. Requiere Arduino CLI con Arduino AVR Boards, bibliotecas hd44780 1.3.2 y FastLED 3.9.20, Python 3 e interfaz compatible con el transporte de la herramienta.

Variables opcionales: `ST01_ARDUINO_CLI`, `ST01_PYTHON`, `ST01_RDM_PORT` y `ST01_RDM_UID`. Por defecto utiliza UID `7ff0:66601337`; sin puerto explícito exige una única interfaz detectada. La utilidad serie incluida soporta una API de interfaz concreta: otras interfaces necesitan su controlador de transporte o un actualizador FTC 1.00 compatible. El firmware del dispositivo no cambia por ello.

En VS Code: **RDM: compilar y cargar controlador**. F5 para controlador también utiliza RDM. La tarea **USB: cargar controlador (respaldo)** queda disponible para recuperación explícita; cerrar el monitor USB antes de utilizarla.

## Cargar un paquete existente

```sh
python3 herramientas/firmware_rdm.py cargar ARCHIVO.st01 --uid 7ff0:66601337 --puerto PUERTO_RDM
```

Usar paquetes ST01 destinados a Mega2560/modelo ST-01. No utilizar un HEX directamente como paquete. El protocolo no actualiza el propio cargador ni equipos externos.

## Recuperación

Las salidas se apagan al entrar en actualización. El cargador valida los bloques y la imagen recibida y grabada antes de aceptar el commit. Dirección y etiqueta se conservan.

No hay doble banco ni copia de la aplicación anterior. Si se pierde alimentación o se interrumpe una carga, el dispositivo permanece en recuperación; volver a alimentar y repetir la carga **completa desde el principio**. No reanuda automáticamente el archivo parcial. La recuperación por RDM después de un corte durante transferencia se ha comprobado en el montaje ST-01. Si el enlace RDM no está disponible, utilizar la ruta USB de respaldo o servicio ISP.

La instalación inicial del cargador se realiza una vez mediante ISP; no se sustituye por una carga normal de aplicación. Véase [mantenimiento del cargador](../arduino/arranque_rdm/LEEME.md).
