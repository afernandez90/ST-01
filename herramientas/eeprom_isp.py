#!/usr/bin/env python3
"""Respaldar/restaurar EEPROM Mega2560 por SPI universal de ArduinoISP.

Evita la traducción de direcciones de EEPROM del modo paginado STK500v1.
No borra flash ni modifica fusibles. La restauración exige respaldo de 4096 bytes.
"""
import argparse
import fcntl
import os
from pathlib import Path
import select
import termios
import time


class ArduinoISP:
    def __init__(self, puerto):
        self.fd = os.open(puerto, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        self.original = termios.tcgetattr(self.fd)
        fcntl.ioctl(self.fd, termios.TIOCEXCL)
        a = termios.tcgetattr(self.fd)
        a[0] = a[1] = a[3] = 0
        a[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        a[4] = a[5] = termios.B19200
        a[6][termios.VMIN] = a[6][termios.VTIME] = 0
        termios.tcsetattr(self.fd, termios.TCSANOW, a)
        termios.tcflush(self.fd, termios.TCIFLUSH)
        self.programando = False

    def comando(self, contenido, cantidad=0):
        datos = bytes(contenido) + b'\x20'
        while datos:
            select.select([], [self.fd], [], 2)
            n = os.write(self.fd, datos)
            datos = datos[n:]
        respuesta = bytearray()
        limite = time.monotonic() + 3
        while len(respuesta) < cantidad + 2:
            restante = limite - time.monotonic()
            if restante <= 0:
                raise TimeoutError('ArduinoISP no responde; comprobar condensador y cableado.')
            if select.select([self.fd], [], [], restante)[0]:
                respuesta.extend(os.read(self.fd, cantidad + 2 - len(respuesta)))
        if respuesta[0] != 0x14 or respuesta[-1] != 0x10:
            raise RuntimeError(f'Respuesta STK500v1 inválida: {respuesta.hex()}')
        return bytes(respuesta[1:-1])

    def spi(self, a, b, c, d):
        return self.comando([0x56, a, b, c, d], 1)[0]

    def iniciar(self):
        # El adaptador USB puede producir un reinicio al abrir el puerto.
        time.sleep(.5)
        for intento in range(3):
            termios.tcflush(self.fd, termios.TCIFLUSH)
            try:
                self.comando([0x30])
                break
            except (RuntimeError, TimeoutError):
                if intento == 2:
                    raise
                time.sleep(.5)
        self.comando([0x50])
        self.programando = True
        firma = bytes(self.spi(0x30, 0, i, 0) for i in range(3))
        if firma != bytes.fromhex('1e9801'):
            raise RuntimeError(f'Firma incorrecta: {firma.hex()}; no se escribe nada.')
        print('ATmega2560 identificado por SPI universal.', flush=True)

    def leer(self, direccion):
        return self.spi(0xa0, direccion >> 8, direccion & 255, 0)

    def respaldo(self):
        salida = bytearray()
        for i in range(4096):
            salida.append(self.leer(i))
            if (i + 1) % 1024 == 0:
                print(f'EEPROM leída: {i + 1}/4096 bytes', flush=True)
        return bytes(salida)

    def restaurar(self, datos):
        if len(datos) != 4096:
            raise ValueError('El respaldo debe contener exactamente 4096 bytes.')
        escritos = 0
        for i, valor in enumerate(datos):
            if self.leer(i) != valor:
                self.spi(0xc0, i >> 8, i & 255, valor)
                time.sleep(.01)
                if self.leer(i) != valor:
                    raise RuntimeError(f'Error de verificación EEPROM en {i}.')
                escritos += 1
        if self.respaldo() != datos:
            raise RuntimeError('La verificación completa de EEPROM ha fallado.')
        print(f'EEPROM restaurada y verificada; {escritos} bytes escritos.', flush=True)

    def cerrar(self):
        try:
            if self.programando:
                self.comando([0x51])
        finally:
            termios.tcsetattr(self.fd, termios.TCSANOW, self.original)
            os.close(self.fd)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--puerto', required=True)
    grupo = p.add_mutually_exclusive_group(required=True)
    grupo.add_argument('--leer', type=Path)
    grupo.add_argument('--restaurar', type=Path)
    a = p.parse_args()
    datos = a.restaurar.read_bytes() if a.restaurar else None
    if datos is not None and len(datos) != 4096:
        p.error('El respaldo debe contener exactamente 4096 bytes.')
    if a.leer and a.leer.exists():
        p.error('El archivo de respaldo ya existe; elija otro nombre.')
    isp = ArduinoISP(a.puerto)
    try:
        isp.iniciar()
        if a.leer:
            a.leer.write_bytes(isp.respaldo())
            print(f'Respaldo guardado en {a.leer}', flush=True)
        else:
            isp.restaurar(datos)
    finally:
        isp.cerrar()


if __name__ == '__main__':
    main()
