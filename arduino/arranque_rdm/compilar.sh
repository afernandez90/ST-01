#!/bin/sh
# Compilación independiente; no programa la placa ni cambia fusibles.
set -eu
raiz=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
datos="$HOME/Library/Arduino15"
if [ ! -d "$datos" ]; then datos="$HOME/.arduino15"; fi
herramientas=${ST01_AVR_TOOLS:-"$datos/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7/bin"}
salida=${ST01_BUILD_DIR:-"$raiz/build/arranque_rdm"}
mkdir -p "$salida"
modo=
if [ "${ST01_RS485_MANUAL:-0}" = 1 ]; then modo=-DST01_RS485_MANUAL; fi
"$herramientas/avr-g++" -mmcu=atmega2560 -DF_CPU=16000000UL -std=gnu++11 -Os -flto \
  $modo -ffunction-sections -fdata-sections -fno-exceptions -fno-threadsafe-statics \
  -Wl,--gc-sections,--undefined=firma,--relax,--section-start=.text=0x3e000,--section-start=.firma=0x3fff0 \
  "$raiz/arduino/arranque_rdm/arranque.cpp" -o "$salida/arranque.elf"
"$herramientas/avr-size" "$salida/arranque.elf"
"$herramientas/avr-objcopy" -O ihex -R .eeprom "$salida/arranque.elf" "$salida/arranque.hex"
/usr/bin/python3 "$raiz/herramientas/verificar_arranque.py" "$salida/arranque.hex"
