#!/bin/sh
# Compilar, empaquetar y actualizar el controlador mediante RDM/FTC.
set -eu
raiz=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cli=${ST01_ARDUINO_CLI:-arduino-cli}
python=${ST01_PYTHON:-python3}
puerto=${ST01_RDM_PORT:-}
uid=${ST01_RDM_UID:-7ff0:66601337}
salida="$raiz/build/controlador"
"$cli" compile --fqbn arduino:avr:mega:cpu=atmega2560 --build-path "$salida" "$raiz/arduino/controlador"
"$python" "$raiz/herramientas/firmware_rdm.py" empaquetar "$salida/controlador.ino.hex" "$salida/ST-01.st01"
set -- "$python" "$raiz/herramientas/firmware_rdm.py" cargar "$salida/ST-01.st01" --uid "$uid"
if [ -n "$puerto" ]; then set -- "$@" --puerto "$puerto"; fi
exec "$@"
