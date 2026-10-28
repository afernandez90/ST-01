#!/usr/bin/env python3
"""Comprobar que el HEX del cargador respeta su región y publica la firma esperada."""
import sys
import re
from pathlib import Path
from firmware_rdm import leer_hex

memoria = leer_hex(sys.argv[1])
if not memoria or min(memoria) != 0x3E000 or max(memoria) >= 0x40000:
    sys.exit('El cargador invade una región no autorizada.')
if bytes(memoria.get(0x3FFF0 + i, 255) for i in range(8)) != b'ST01RDM1':
    sys.exit('Falta la firma del cargador en la dirección esperada.')
print('Cargador dentro de los 8 KiB reservados; firma verificada.')

if len(sys.argv) > 2:
    texto = Path(sys.argv[2]).read_text()
    inicio = re.search(r'^([0-9a-f]+) <desactivarWatchdog>:\n(.*?)(?=^\s*$)', texto, re.M | re.S)
    datos = re.search(r'^([0-9a-f]+) <__do_copy_data>:', texto, re.M)
    if not inicio or not datos or int(inicio[1], 16) >= int(datos[1], 16):
        sys.exit('Falta la desactivación temprana del watchdog antes de inicializar RAM.')
    cuerpo = inicio[2]
    if not re.search(r'\bwdr\b', cuerpo) or not re.search(r'sts\s+0x0060, r1\b', cuerpo):
        sys.exit('La rutina temprana no desactiva WDTCSR como se esperaba.')
    print('Desactivación temprana del watchdog conservada con LTO y verificada.')
