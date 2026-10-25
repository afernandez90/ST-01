#!/usr/bin/env python3
"""Comprobar que el HEX del cargador respeta su región y publica la firma esperada."""
import sys
from firmware_rdm import leer_hex

memoria = leer_hex(sys.argv[1])
if not memoria or min(memoria) != 0x3E000 or max(memoria) >= 0x40000:
    sys.exit('El cargador invade una región no autorizada.')
if bytes(memoria.get(0x3FFF0 + i, 255) for i in range(8)) != b'ST01RDM1':
    sys.exit('Falta la firma del cargador en la dirección esperada.')
print('Cargador dentro de los 8 KiB reservados; firma verificada.')
