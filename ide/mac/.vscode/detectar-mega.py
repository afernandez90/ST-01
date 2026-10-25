"""Selecciona un único Mega a partir de la salida JSON de Arduino CLI."""
import json
import sys

datos = json.load(sys.stdin)
puertos = sorted({
    entrada["port"]["address"]
    for entrada in datos.get("detected_ports", [])
    if entrada.get("port", {}).get("protocol") == "serial"
    and any(
        placa.get("fqbn", "").split(":")[:3] == ["arduino", "avr", "mega"]
        for placa in entrada.get("matching_boards", [])
    )
})
if len(puertos) != 1:
    mensaje = (
        "No se detecta un Arduino Mega. Comprueba el cable USB y vuelve a intentarlo."
        if not puertos else
        "Hay varios Mega conectados: " + ", ".join(puertos)
        + ". Desconecta los demás o define ST01_PORT."
    )
    print(mensaje, file=sys.stderr)
    sys.exit(1)
print(puertos[0])
