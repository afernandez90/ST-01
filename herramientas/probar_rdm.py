#!/usr/bin/env python3
"""Probar el respondedor ST-01; --completo cambia y restaura sus parámetros."""
import argparse
import glob
import fcntl
import json
import os
import select
import struct
import termios
import time
from firmware_rdm import Enttec, trama_rdm, uid


def descubrir(t, destino):
    t.transaccion = (t.transaccion + 1) & 255
    # Intervalo inclusivo de un UID: no interfiere con otros equipos del bus.
    p = trama_rdm(bytes([255]) * 6, 0x10, 1, destino + destino,
                  t.transaccion, t.uid_controlador)
    t.enviar(11, p)
    fin = time.monotonic() + 1
    while time.monotonic() < fin:
        try:
            etiqueta, p = t.recibir(fin)
        except TimeoutError:
            return None
        if etiqueta != 5 or not p or p[0]:
            continue
        r = p[1:]
        i = r.find(b'\xaa')
        if i < 0 or len(r) < i + 17:
            continue
        codificado = r[i + 1:i + 17]
        decodificado = bytes(codificado[j] & codificado[j + 1] for j in range(0, 16, 2))
        if sum(codificado[:12]) & 0xffff != int.from_bytes(decodificado[6:], 'big'):
            raise ValueError('Suma incorrecta en la respuesta de descubrimiento.')
        return decodificado[:6]
    return None


def probar(t, completo=False, mega=None):
    informe = {'uid_dispositivo': t.destino.hex(), 'uid_controlador': t.uid_controlador.hex()}
    get = lambda pid, p=b'': t.comando(0x20, pid, p)
    set_ = lambda pid, p: t.comando(0x30, pid, p)
    t.comando(0x10, 3, b'')
    try:
        assert descubrir(t, t.destino) == t.destino, 'No aparece el UID esperado.'
        t.comando(0x10, 2, b'')
        assert descubrir(t, t.destino) is None, 'El equipo no silencia descubrimiento.'
    finally:
        t.comando(0x10, 3, b'')
    assert descubrir(t, t.destino) == t.destino
    informe['descubrimiento_silencio'] = True
    info = get(0x60)
    assert len(info) == 19 and int.from_bytes(info[10:12], 'big') == 19
    direccion = get(0xf0)
    etiqueta = get(0x82)
    identificar = get(0x1000)
    personalidad = get(0xe0)
    descripcion = get(0xe1, b'\x01')
    parametros = get(0x50)
    informe.update({
        'firmware': get(0xc0).decode('ascii'),
        'modelo': get(0x80).decode('ascii'),
        'fabricante': get(0x81).decode('ascii'),
        'etiqueta': etiqueta.decode('ascii', 'replace'),
        'direccion_original': int.from_bytes(direccion, 'big'),
        'canales': int.from_bytes(info[10:12], 'big'),
        'personalidad': descripcion[3:].decode('ascii'),
        'ftc_disponible': b'\x12\x00' in [parametros[i:i+2] for i in range(0, len(parametros), 2)]})
    assert personalidad == b'\x01\x01' and descripcion[:3] == b'\x01\x00\x13'
    assert int.from_bytes(info[14:16], 'big') == informe['direccion_original']
    if completo:
        temporal = 2 if direccion != b'\x00\x02' else 3
        temporal = temporal.to_bytes(2, 'big')
        try:
            set_(0x1000, b'\x01')
            assert get(0x1000) == b'\x01'
            time.sleep(2)
            set_(0x1000, b'\0')
            assert get(0x1000) == b'\0'
            informe['identificacion_get_set'] = True
            set_(0x82, b'ST-01 prueba RDM')
            assert get(0x82) == b'ST-01 prueba RDM'
            set_(0xf0, temporal)
            assert get(0xf0) == temporal
            time.sleep(0.3)
            if mega:
                fd = os.open(mega, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
                original = termios.tcgetattr(fd)
                try:
                    fcntl.ioctl(fd, termios.TIOCEXCL)
                    ajustes = termios.tcgetattr(fd)
                    ajustes[0] = ajustes[1] = ajustes[3] = 0
                    ajustes[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
                    ajustes[4] = ajustes[5] = termios.B115200
                    ajustes[6][termios.VMIN] = ajustes[6][termios.VTIME] = 0
                    termios.tcsetattr(fd, termios.TCSANOW, ajustes)
                    termios.tcflush(fd, termios.TCIFLUSH)
                    fcntl.ioctl(fd, termios.TIOCMBIC, struct.pack('I', termios.TIOCM_DTR))
                    time.sleep(0.1)
                    fcntl.ioctl(fd, termios.TIOCMBIS, struct.pack('I', termios.TIOCM_DTR))
                    arranque = b''
                    fin = time.monotonic() + 3
                    while time.monotonic() < fin:
                        if select.select([fd], [], [], max(0, fin-time.monotonic()))[0]:
                            arranque += os.read(fd, 4096)
                    assert b'[INICIO]' in arranque, 'No se ha confirmado el reinicio por USB.'
                finally:
                    termios.tcsetattr(fd, termios.TCSANOW, original)
                    os.close(fd)
                assert get(0xf0) == temporal
                assert get(0x82) == b'ST-01 prueba RDM'
                informe['persistencia_tras_reinicio_usb'] = True
            for invalida in (0, 495):
                try:
                    set_(0xf0, invalida.to_bytes(2, 'big'))
                except ValueError as error:
                    assert str(error).endswith('0006')
                else:
                    raise AssertionError('Se ha aceptado una dirección fuera de rango.')
            assert get(0xf0) == temporal
            informe['direccion_get_set_y_limites'] = True
            informe['etiqueta_get_set'] = True
        finally:
            # Restaurar incluso si falla una comprobación intermedia.
            errores = []
            for pid, valor in ((0x1000, identificar), (0xf0, direccion), (0x82, etiqueta)):
                try:
                    set_(pid, valor)
                except Exception as error:
                    errores.append(str(error))
            time.sleep(0.3)
            if errores:
                raise RuntimeError('No se ha podido restaurar: ' + '; '.join(errores))
        assert get(0xf0) == direccion and get(0x82) == etiqueta and get(0x1000) == identificar
        informe['configuracion_restaurada'] = True
    return informe


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--puerto')
    parser.add_argument('--uid', default='7ff0:66601337')
    parser.add_argument('--completo', action='store_true')
    parser.add_argument('--mega', help='Puerto USB del Mega para comprobar persistencia al reiniciar')
    parser.add_argument('--informe', help='Guardar el informe JSON')
    args = parser.parse_args()
    puertos = [args.puerto] if args.puerto else glob.glob('/dev/cu.usbserial-*')
    if len(puertos) != 1:
        parser.error('Especifique --puerto; no hay una única interfaz serie.')
    t = Enttec(puertos[0], uid(args.uid))
    try:
        informe = probar(t, args.completo, args.mega)
    finally:
        t.close()
    texto = json.dumps(informe, ensure_ascii=False, indent=2)
    print(texto)
    if args.informe:
        with open(args.informe, 'w') as archivo:
            archivo.write(texto + '\n')


if __name__ == '__main__':
    main()
