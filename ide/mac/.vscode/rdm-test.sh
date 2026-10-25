#!/bin/sh
# Lanzador local: pruebas simuladas y pruebas físicas mediante una interfaz RDM compatible.
set -eu
raiz=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
seleccion=${1:-todas}
case "$seleccion" in
  todas) exec /bin/sh "$raiz/pruebas/rdm.sh" ;;
  firmware) exec /usr/bin/python3 -B "$raiz/pruebas/firmware_rdm.py" ;;
  fisica|fisica-completa|fisica-persistencia)
    set -- /usr/bin/python3 -B "$raiz/herramientas/probar_rdm.py"
    if [ "$seleccion" != fisica ]; then set -- "$@" --completo; fi
    if [ "$seleccion" = fisica-persistencia ]; then
      mega=$(/opt/homebrew/bin/arduino-cli board list --format json | /usr/bin/python3 "$raiz/.vscode/detectar-mega.py")
      set -- "$@" --mega "$mega"
    fi
    echo 'Cerrar las aplicaciones que usen la interfaz RDM; cerrar el monitor USB para reiniciar el Mega.'
    exec "$@" ;;
  rdm) fuente=rdm.cpp ;;
  recepcion) fuente=recepcion_rdm.cpp ;;
  ftc) fuente=ftc.cpp ;;
  traza) fuente=traza_rdm.cpp ;;
  *) echo "Prueba desconocida: $seleccion" >&2; exit 2 ;;
esac
carpeta=$(mktemp -d "${TMPDIR:-/tmp}/st01-rdm-individual.XXXXXX")
trap 'rm -rf "$carpeta"' EXIT
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I"$raiz/pruebas/simulacion" "$raiz/pruebas/$fuente" -o "$carpeta/prueba"
"$carpeta/prueba"
