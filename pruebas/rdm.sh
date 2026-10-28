#!/bin/sh
# Compilar las pruebas fuera del repositorio; no requiere bibliotecas externas.
set -eu
raiz=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
carpeta=$(mktemp -d "${TMPDIR:-/tmp}/st01-rdm.XXXXXX")
trap 'rm -rf "$carpeta"' EXIT
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  "$raiz/pruebas/rdm.cpp" -o "$carpeta/rdm"
"$carpeta/rdm"
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I"$raiz/pruebas/simulacion" "$raiz/pruebas/recepcion_rdm.cpp" -o "$carpeta/recepcion"
"$carpeta/recepcion"
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  "$raiz/pruebas/ftc.cpp" -o "$carpeta/ftc"
"$carpeta/ftc"
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  "$raiz/pruebas/efectos.cpp" -o "$carpeta/efectos"
"$carpeta/efectos"
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  "$raiz/pruebas/digitos.cpp" -o "$carpeta/digitos"
"$carpeta/digitos"
PYTHONDONTWRITEBYTECODE=1 /usr/bin/python3 "$raiz/pruebas/firmware_rdm.py"
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I"$raiz/pruebas/simulacion" "$raiz/pruebas/traza_rdm.cpp" -o "$carpeta/traza"
"$carpeta/traza"

c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  "$raiz/pruebas/usb_arranque.cpp" -o "$carpeta/usb_arranque"
"$carpeta/usb_arranque"
PYTHONDONTWRITEBYTECODE=1 /usr/bin/python3 "$raiz/pruebas/eeprom_isp.py"
