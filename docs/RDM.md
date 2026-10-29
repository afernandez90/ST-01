# Gestión RDM — ST-01 v1.0.0

ST-01 mantiene 19 canales y una personalidad **Control v1.0.0**, con dirección base 1–494. RDM permite descubrir el equipo, consultar información, cambiar dirección/etiqueta, identificarlo y actualizar su aplicación.

## Conexión e identidad

El retorno necesita transceptor RS-485 bidireccional: recepción TTL a Mega 19/RX1 y transmisión desde 18/TX1. Véase [cableado](CABLEADO.md). No utilizar Serial1 desde otra biblioteca mientras el controlador gestiona esta UART.

UID de esta unidad: **7FF0:66601337**. Es independiente de la dirección DMX y se conserva tras reiniciar. El identificador 7FF0 pertenece al rango experimental de ESTA; no es un identificador comercial registrado. Para otra unidad en el mismo bus, asignar un número de dispositivo diferente en `arduino/controlador/identidad.h`. La versión 1.0.0 no implica certificación RDM ni asignación comercial de fabricante.

## Operaciones

| Parámetro | Comportamiento |
|---|---|
| DISC_UNIQUE_BRANCH | Descubrimiento por intervalo de UID |
| DISC_MUTE / DISC_UN_MUTE | Silenciar/restaurar descubrimiento |
| DEVICE_INFO | Modelo 1, firmware 1.0.0, 19 canales, una personalidad |
| DEVICE_MODEL_DESCRIPTION | ST-01 Control v1.0.0 |
| MANUFACTURER_LABEL | Salto Temporal |
| DEVICE_LABEL | Etiqueta de usuario, persistente |
| SOFTWARE_VERSION_LABEL | 1.0.0 RDM |
| DMX_START_ADDRESS | GET/SET, 1–494, persistente |
| DMX_PERSONALITY / DESCRIPTION | Única personalidad Control v1.0.0 |
| IDENTIFY_DEVICE | GET/SET, identificación visual |
| SUPPORTED_PARAMETERS | Lista de parámetros admitidos |
| FTC 0x1200–0x1203 | Actualización cuando el cargador compatible está instalado |

## Identificar

IDENTIFY_DEVICE=1 sustituye temporalmente DMX: pilotos, tiras en blanco y los cuatro dígitos mostrando `8.` parpadean juntos; también la iluminación del LCD. Al desactivar identificación se recupera el DMX vigente; sin señal válida se apagan las salidas. No modifica escenas, dirección ni etiqueta. El arranque realiza por sí mismo dos destellos, sin necesitar RDM.

Dirección y etiqueta se guardan en EEPROM. Tras cambiar dirección, ajustar también la consola al nuevo inicio. No se implementan sensores, control directo de intensidades por RDM ni subdispositivos. Para el procedimiento de actualización y recuperación, consultar [actualización RDM](ACTUALIZACION_RDM.md).
