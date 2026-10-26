#!/bin/sh
# Lanzador del controlador y las pruebas del Mega desde VS Code.
set -eu
raiz=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cli=/opt/homebrew/bin/arduino-cli
placa=arduino:avr:mega:cpu=atmega2560
accion=${ST01_ACTION:-compile}
prueba=${ST01_SKETCH:-controlador}
puerto=${ST01_PORT:-}

case "$accion" in
  list) exec "$cli" board list ;;
  compile|upload|monitor) ;;
  *) echo "Acción desconocida: $accion" >&2; exit 1 ;;
esac
if [ -n "${ST01_SKETCH_DIR:-}" ]; then
  carpeta=$(CDPATH= cd -- "$ST01_SKETCH_DIR" && pwd)
  prueba=$(basename -- "$carpeta")
else
  case "$prueba" in
    controlador|escaner_i2c|parpadeo) ;;
    *) echo "Prueba desconocida: $prueba" >&2; exit 1 ;;
  esac
  if [ "$prueba" = controlador ]; then
    carpeta="$raiz/arduino/controlador"
  else
    carpeta="$raiz/arduino/pruebas/$prueba"
  fi
fi
if [ ! -f "$carpeta/$prueba.ino" ]; then
  echo "Abre un archivo .ino de un sketch Arduino antes de ejecutar esta tarea." >&2
  exit 1
fi
if [ "$accion" = upload ] || [ "$accion" = monitor ]; then
  if [ -z "$puerto" ]; then
    dispositivos=$("$cli" board list --json --discovery-timeout 3s)
    puerto=$(printf '%s' "$dispositivos" | /usr/bin/python3 "$raiz/.vscode/detectar-mega.py")
  fi
  if [ ! -c "$puerto" ]; then
    echo "El puerto serie no está disponible: $puerto" >&2
    exit 1
  fi
  echo "Arduino Mega: $puerto"
fi
case "$accion" in
  compile)
    exec "$cli" compile --fqbn "$placa" --build-path "$raiz/build/$prueba" "$carpeta" ;;
  upload)
    "$cli" compile --fqbn "$placa" --build-path "$raiz/build/$prueba" --port "$puerto" --upload "$carpeta"
    echo "Carga completada. Monitor USB a 115200 baudios; Ctrl+C para cerrar."
    exec "$cli" monitor --port "$puerto" --config baudrate=115200 ;;
  monitor)
    exec "$cli" monitor --port "$puerto" --config baudrate=115200 ;;
esac
