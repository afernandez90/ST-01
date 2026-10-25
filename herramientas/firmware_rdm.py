#!/usr/bin/env python3
"""Empaquetar firmware Mega y transferirlo mediante FTC 1.00 / RDM."""
import argparse
import binascii
import glob
import os
from pathlib import Path
import secrets
import select
import struct
import sys
import time

LIMITE = 0x3E000
UID_CONTROLADOR = bytes.fromhex('7ff0fffffffe')


def crc16(datos):
    crc = 0xFFFF
    for b in datos:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ (0xA001 if crc & 1 else 0)
    return crc


def leer_hex(ruta):
    memoria, base, final = {}, 0, False
    for linea in Path(ruta).read_text().splitlines():
        if not linea.strip():
            continue
        if final or not linea.startswith(':'):
            raise ValueError('Archivo HEX inválido o datos después del final.')
        p = bytes.fromhex(linea[1:])
        if len(p) < 5 or len(p) != p[0] + 5 or sum(p) & 255:
            raise ValueError('Longitud o suma incorrecta en el HEX.')
        n, direccion, tipo = p[0], int.from_bytes(p[1:3], 'big'), p[3]
        datos = p[4:-1]
        if tipo == 0:
            for i, b in enumerate(datos):
                pos = base + direccion + i
                if pos in memoria and memoria[pos] != b:
                    raise ValueError('Direcciones solapadas en el HEX.')
                memoria[pos] = b
        elif tipo == 1 and n == 0:
            final = True
        elif tipo in (2, 4) and n == 2:
            base = int.from_bytes(datos, 'big') << (4 if tipo == 2 else 16)
        elif tipo not in (3, 5):
            raise ValueError('Tipo de registro HEX no admitido.')
    if not final:
        raise ValueError('Falta el registro final del HEX.')
    return memoria


def empaquetar(ruta):
    memoria = leer_hex(ruta)
    if not memoria or min(memoria) != 0 or max(memoria) >= LIMITE:
        raise ValueError('Se requiere una aplicación Mega sin cargador y con dirección inicial cero.')
    imagen = bytes(memoria.get(i, 255) for i in range(max(memoria) + 1))
    if len(imagen) < 4:
        raise ValueError('Imagen de aplicación incompleta.')
    cabecera = b'ST01\x01\x25\x00\x01' + struct.pack('>II', len(imagen), binascii.crc32(imagen))
    return cabecera + imagen


def validar_archivo(datos):
    if len(datos) < 20 or len(datos) > LIMITE + 16 or datos[:8] != b'ST01\x01\x25\x00\x01':
        raise ValueError('Paquete incompatible con ST-01 / Mega2560.')
    tamano, esperado = struct.unpack('>II', datos[8:16])
    if tamano != len(datos) - 16 or binascii.crc32(datos[16:]) != esperado:
        raise ValueError('Tamaño o CRC32 de la imagen incorrecto.')


def uid(texto):
    valor = bytes.fromhex(texto.replace(':', ''))
    if len(valor) != 6 or valor[2:] == b'\xff' * 4:
        raise ValueError('Se requiere un UID individual de seis bytes.')
    return valor


def trama_rdm(destino, clase, pid, datos, transaccion, controlador=UID_CONTROLADOR):
    if len(datos) > 231:
        raise ValueError('Demasiados datos RDM.')
    p = (bytes((0xCC, 1, 24 + len(datos))) + destino + controlador +
         bytes((transaccion, 1, 0, 0, 0, clase)) + struct.pack('>HB', pid, len(datos)) + datos)
    return p + struct.pack('>H', sum(p) & 0xFFFF)


def uid_enttec(serie):
    if len(serie) != 4:
        raise ValueError('Número de serie ENTTEC incompleto.')
    return bytes.fromhex('454e') + serie[::-1]


class Enttec:
    """Transporte del controlador; el cargador Arduino no depende de ENTTEC."""
    def __init__(self, puerto, destino):
        import fcntl
        import termios
        self.t, self.destino, self.transaccion = termios, destino, 0
        self.fd = os.open(puerto, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        self.original = termios.tcgetattr(self.fd)
        try:
            fcntl.ioctl(self.fd, termios.TIOCEXCL)
            ajustes = termios.tcgetattr(self.fd)
            ajustes[0] = ajustes[1] = ajustes[3] = 0
            ajustes[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
            ajustes[4] = ajustes[5] = termios.B57600
            ajustes[6][termios.VMIN] = ajustes[6][termios.VTIME] = 0
            termios.tcsetattr(self.fd, termios.TCSANOW, ajustes)
            termios.tcflush(self.fd, termios.TCIFLUSH)
            self.buffer = b''
            self.enviar(3, b'\0\0')
            etiqueta, p = self.recibir(time.monotonic() + 2)
            if etiqueta != 3 or len(p) < 5 or p[1] != 2:
                raise ValueError('El ENTTEC necesita firmware DMX/RDM 2.x.')
            self.enviar(10, b'')
            etiqueta, serie = self.recibir(time.monotonic() + 2)
            if etiqueta != 10 or len(serie) != 4:
                raise ValueError('No se puede obtener el UID propio del ENTTEC.')
            # El firmware filtra las respuestas normales por su propio UID.
            self.uid_controlador = uid_enttec(serie)
        except BaseException:
            self.close()
            raise

    def close(self):
        try:
            self.t.tcsetattr(self.fd, self.t.TCSANOW, self.original)
        finally:
            os.close(self.fd)

    def enviar(self, etiqueta, datos):
        p = bytes((0x7E, etiqueta)) + struct.pack('<H', len(datos)) + datos + b'\xe7'
        while p:
            try:
                n = os.write(self.fd, p)
                p = p[n:]
            except BlockingIOError:
                select.select([], [self.fd], [], 1)

    def recibir(self, fin):
        while time.monotonic() < fin:
            while self.buffer:
                i = self.buffer.find(b'\x7e')
                if i < 0:
                    self.buffer = b''
                    break
                self.buffer = self.buffer[i:]
                if len(self.buffer) < 4:
                    break
                n = int.from_bytes(self.buffer[2:4], 'little')
                if n > 600:
                    self.buffer = self.buffer[1:]
                    continue
                if len(self.buffer) < n + 5:
                    break
                p, self.buffer = self.buffer[:n + 5], self.buffer[n + 5:]
                if p[-1] == 0xE7:
                    return p[1], p[4:-1]
            if select.select([self.fd], [], [], max(0, fin - time.monotonic()))[0]:
                self.buffer += os.read(self.fd, 4096)
        raise TimeoutError('El dispositivo no responde por RDM.')

    def comando(self, clase, pid, datos):
        for intento in range(3):
            self.transaccion = (self.transaccion + 1) & 255
            p = trama_rdm(self.destino, clase, pid, datos, self.transaccion, self.uid_controlador)
            self.enviar(7, p)
            fin = time.monotonic() + 2
            try:
                while True:
                    etiqueta, recibido = self.recibir(fin)
                    if etiqueta != 5 or not recibido or recibido[0] != 0:
                        continue
                    r = recibido[1:]
                    if (len(r) < 26 or r[:2] != b'\xcc\x01' or len(r) != r[2] + 2 or
                            r[23] != r[2] - 24 or sum(r[:-2]) & 0xFFFF != int.from_bytes(r[-2:], 'big')):
                        continue
                    if (r[3:9] != self.uid_controlador or r[9:15] != self.destino or
                            r[15] != self.transaccion or r[20] != clase + 1 or
                            int.from_bytes(r[21:23], 'big') != pid):
                        continue
                    if r[16] != 0:
                        raise ValueError('Respuesta RDM de error: ' + r[24:-2].hex())
                    return r[24:-2]
            except TimeoutError:
                if intento == 2:
                    raise
                time.sleep(0.1)


def estado(respuesta):
    if len(respuesta) < 5:
        raise ValueError('Respuesta FTC incompleta.')
    return respuesta[0], int.from_bytes(respuesta[1:5], 'big')


def cargar(transporte, archivo):
    validar_archivo(archivo)
    sesion = secrets.randbelow(254) + 1
    iniciar = (bytes((sesion, 1)) + struct.pack('>HHI', 0x100, 0, len(archivo)) +
               archivo[:16] + struct.pack('>H', crc16(archivo)))
    iniciar += struct.pack('>H', crc16(iniciar))
    r = transporte.comando(0x30, 0x1200, iniciar)
    codigo, espera = estado(r)
    if codigo == 6:
        time.sleep(espera / 1000)
        r = transporte.comando(0x30, 0x1200, iniciar)
        codigo, espera = estado(r)
    if codigo != 1 or len(r) != 38 or r[5] != sesion or r[6] != 1:
        raise ValueError(f'No se puede iniciar FTC (estado {codigo}). ¿Está instalado el cargador?')
    if int.from_bytes(r[7:9], 'big') != 0x100 or not 1 <= r[21] <= 224:
        raise ValueError('Versión o tamaño de bloque FTC no compatible.')
    bloque, inicial, intervalo = r[21], int.from_bytes(r[22:26], 'big'), int.from_bytes(r[26:28], 'big')
    acumulados, demora_acumulada = int.from_bytes(r[28:32], 'big'), int.from_bytes(r[32:34], 'big')
    validacion = int.from_bytes(r[34:36], 'big')
    time.sleep(inicial / 1000)
    offset, cuenta = 0, 0
    while offset < len(archivo):
        datos = archivo[offset:offset + bloque]
        p = bytes((sesion,)) + struct.pack('>I', offset) + datos
        p += struct.pack('>H', crc16(p))
        for intento in range(3):
            r = transporte.comando(0x30, 0x1201, p)
            codigo, espera = estado(r)
            if codigo != 11:
                break
        if codigo == 11:
            raise ValueError('Tres errores CRC en el mismo bloque.')
        limite = time.monotonic() + 60
        while codigo == 3 and time.monotonic() < limite:
            time.sleep(espera / 1000)
            r = transporte.comando(0x20, 0x1201, bytes((sesion,)))
            codigo, espera = estado(r)
        if codigo not in (0, 4) or len(r) != 9 or int.from_bytes(r[5:9], 'big') != offset + len(datos):
            raise ValueError(f'Error de transferencia FTC: {codigo}. Se conserva el modo de recuperación.')
        offset += len(datos)
        cuenta += len(datos)
        print(f'\rTransferidos {offset}/{len(archivo)} bytes', end='', flush=True)
        time.sleep(intervalo / 1000)
        if acumulados and cuenta >= acumulados:
            time.sleep(demora_acumulada / 1000)
            cuenta = 0
    print()
    time.sleep(validacion / 1000)
    p = bytes((sesion, 0, 0))
    r = transporte.comando(0x20, 0x1202, p)
    codigo, espera = estado(r)
    limite = time.monotonic() + 120
    while codigo == 3 and time.monotonic() < limite:
        time.sleep(espera / 1000)
        r = transporte.comando(0x20, 0x1202, p)
        codigo, espera = estado(r)
    if codigo != 0 or len(r) != 15 or int.from_bytes(r[7:9], 'big') != crc16(archivo):
        raise ValueError(f'Validación FTC fallida: {codigo}. No se confirma la imagen.')
    r = transporte.comando(0x30, 0x1202, p)
    codigo, espera = estado(r)
    if codigo != 0:
        raise ValueError(f'Confirmación FTC fallida: {codigo}.')
    time.sleep(espera / 1000)
    version = transporte.comando(0x20, 0x00C0, b'')
    print('Firmware actualizado. Versión reportada:', version.decode('ascii', 'replace'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='accion', required=True)
    pack = sub.add_parser('empaquetar', help='Crear un paquete ST01 desde un HEX de aplicación')
    pack.add_argument('hex'); pack.add_argument('salida')
    upload = sub.add_parser('cargar', help='Actualizar mediante un ENTTEC Pro con firmware RDM')
    upload.add_argument('archivo'); upload.add_argument('--uid', default='7ff0:66601337')
    upload.add_argument('--puerto')
    args = parser.parse_args()
    if args.accion == 'empaquetar':
        datos = empaquetar(args.hex); validar_archivo(datos)
        Path(args.salida).write_bytes(datos)
        print(f'Paquete creado: {len(datos)} bytes, CRC FTC {crc16(datos):04X}')
    else:
        puertos = [args.puerto] if args.puerto else glob.glob('/dev/cu.usbserial-*')
        if len(puertos) != 1:
            raise ValueError('Especifique --puerto: no se ha detectado una única interfaz serie.')
        datos = Path(args.archivo).read_bytes(); validar_archivo(datos)
        transporte = Enttec(puertos[0], uid(args.uid))
        try:
            cargar(transporte, datos)
        finally:
            transporte.close()


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, TimeoutError) as error:
        sys.exit(str(error))
