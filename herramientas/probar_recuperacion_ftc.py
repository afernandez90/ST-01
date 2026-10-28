#!/usr/bin/env python3
"""Prueba física FTC: CRC, duplicados, interrupción de enlace y recarga completa.

Escribe parte del firmware indicado y después lo carga completo. No simula un corte
eléctrico. Requiere un archivo ST01 válido y una interfaz compatible con el transporte.
"""
import argparse
import json
from pathlib import Path
import secrets
import struct
import time
from firmware_rdm import Enttec, cargar, crc16, estado, uid, validar_archivo


def iniciar(t, archivo, sesion):
    p = (bytes((sesion, 1)) + struct.pack('>HHI', 0x100, 0, len(archivo)) +
         archivo[:16] + struct.pack('>H', crc16(archivo)))
    p += struct.pack('>H', crc16(p))
    r = t.comando(0x30, 0x1200, p)
    codigo, espera = estado(r)
    if codigo == 6:
        time.sleep(espera / 1000)
        r = t.comando(0x30, 0x1200, p)
        codigo, _ = estado(r)
    if codigo != 1 or len(r) != 38 or r[5] != sesion or r[21] != 128:
        raise ValueError('No se ha iniciado la sesión de prueba FTC.')
    time.sleep(int.from_bytes(r[22:26], 'big') / 1000)


def bloque(archivo, sesion, offset):
    p = bytes((sesion,)) + struct.pack('>I', offset) + archivo[offset:offset + 128]
    return p + struct.pack('>H', crc16(p))


def esperar(t, sesion, r):
    limite = time.monotonic() + 10
    while estado(r)[0] == 3:
        if time.monotonic() >= limite:
            raise TimeoutError('El cargador no termina de procesar el bloque.')
        time.sleep(max(.02, estado(r)[1] / 1000))
        r = t.comando(0x20, 0x1201, bytes((sesion,)))
    return r


def probar(puerto, destino, archivo):
    validar_archivo(archivo)
    t = Enttec(puerto, destino)
    informe = {}
    iniciado = False
    try:
        direccion = t.comando(0x20, 0xf0, b'')
        etiqueta = t.comando(0x20, 0x82, b'')
        sesion = secrets.randbelow(254) + 1
        iniciar(t, archivo, sesion)
        iniciado = True
        p = bloque(archivo, sesion, 0)
        r = t.comando(0x30, 0x1201, p[:-1] + bytes((p[-1] ^ 1,)))
        if estado(r)[0] != 11:
            raise ValueError('El cargador no rechaza el CRC de bloque incorrecto.')
        informe['crc_incorrecto_rechazado'] = True
        for offset in (0, 128, 256):
            p = bloque(archivo, sesion, offset)
            r = esperar(t, sesion, t.comando(0x30, 0x1201, p))
            if estado(r)[0] != 0 or int.from_bytes(r[5:9], 'big') != offset + 128:
                raise ValueError('Desplazamiento incorrecto tras el bloque.')
        r = t.comando(0x30, 0x1201, p)
        if estado(r)[0] != 0 or int.from_bytes(r[5:9], 'big') != 384:
            raise ValueError('El reenvío duplicado no es idempotente.')
        informe['bloque_duplicado_correcto'] = True
        r = t.comando(0x30, 0x1201, bloque(archivo, sesion, 512))
        if estado(r)[0] != 0x14 or estado(r)[1] != 384:
            raise ValueError('No se rechaza el desplazamiento fuera de secuencia.')
        informe['desplazamiento_incorrecto_rechazado'] = True
        t.close()
        t = None
        time.sleep(.5)
        t = Enttec(puerto, destino)
        version = t.comando(0x20, 0xc0, b'').decode('ascii', 'replace')
        if 'cargador' not in version:
            raise ValueError('El equipo no permanece en el cargador durante la interrupción.')
        r = t.comando(0x20, 0x1201, bytes((sesion,)))
        if estado(r)[0] != 0 or int.from_bytes(r[5:9], 'big') != 384:
            raise ValueError('Se pierde el estado de la sesión al reconectar el enlace.')
        informe['interrupcion_enlace_y_reconexion'] = True
    finally:
        if iniciado:
            if t is None:
                t = Enttec(puerto, destino)
            try:
                cargar(t, archivo)
            finally:
                t.close()
        elif t is not None:
            t.close()
    # Carga terminada: comprobar configuración sin cambiarla.
    t = Enttec(puerto, destino)
    try:
        if t.comando(0x20, 0xf0, b'') != direccion or t.comando(0x20, 0x82, b'') != etiqueta:
            raise ValueError('La configuración cambió durante la actualización.')
        informe['direccion_y_etiqueta_preservadas'] = True
        informe['recarga_completa_y_reinicio'] = True
        informe['firmware'] = t.comando(0x20, 0xc0, b'').decode('ascii', 'replace')
        return informe
    finally:
        t.close()


def preparar_corte(puerto, destino, archivo):
    """Dejar una carga parcial pendiente para comprobar un corte físico posterior."""
    validar_archivo(archivo)
    t = Enttec(puerto, destino)
    try:
        direccion = t.comando(0x20, 0xf0, b'')
        etiqueta = t.comando(0x20, 0x82, b'')
        sesion = secrets.randbelow(254) + 1
        iniciar(t, archivo, sesion)
        for offset in (0, 128, 256):
            r = esperar(t, sesion, t.comando(0x30, 0x1201, bloque(archivo, sesion, offset)))
            if estado(r)[0] != 0 or int.from_bytes(r[5:9], 'big') != offset + 128:
                raise ValueError('No se ha preparado la carga parcial esperada.')
        return {'estado': 'carga parcial; pendiente corte eléctrico y recarga completa',
                'bytes_transferidos': 384, 'sesion': sesion,
                'direccion_original': int.from_bytes(direccion, 'big'),
                'etiqueta_original': etiqueta.decode('ascii', 'replace')}
    finally:
        t.close()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('archivo', type=Path)
    p.add_argument('--puerto', required=True)
    p.add_argument('--uid', default='7ff0:66601337')
    p.add_argument('--informe', type=Path, required=True)
    p.add_argument('--preparar-corte', action='store_true',
                   help='Dejar carga parcial pendiente para un corte físico; después ejecutar cargar del firmware.')
    a = p.parse_args()
    funcion = preparar_corte if a.preparar_corte else probar
    informe = funcion(a.puerto, uid(a.uid), a.archivo.read_bytes())
    a.informe.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n')
    print(json.dumps(informe, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
