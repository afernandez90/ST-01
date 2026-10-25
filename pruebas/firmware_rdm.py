"""Pruebas del paquete y del controlador FTC sin acceder a USB."""
import importlib.util
import contextlib
import io
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

ruta = Path(__file__).resolve().parents[1] / 'herramientas' / 'firmware_rdm.py'
spec = importlib.util.spec_from_file_location('firmware', ruta)
f = importlib.util.module_from_spec(spec)
spec.loader.exec_module(f)


class PruebasFirmware(unittest.TestCase):
    def paquete(self):
        with tempfile.TemporaryDirectory() as carpeta:
            p = Path(carpeta) / 'programa.hex'
            p.write_text(':040000000C9400015B\n:00000001FF\n')
            return f.empaquetar(p)

    def test_crc_paquete_y_hex(self):
        self.assertEqual(f.crc16(b'123456789'), 0x4B37)
        p = self.paquete()
        f.validar_archivo(p)
        self.assertEqual(p[16:], b'\x0c\x94\0\x01')
        with self.assertRaises(ValueError):
            f.validar_archivo(p[:-1] + b'\0')
        with tempfile.TemporaryDirectory() as carpeta:
            p = Path(carpeta) / 'invalido.hex'
            p.write_text(':040000000C94000100\n:00000001FF\n')
            with self.assertRaises(ValueError):
                f.empaquetar(p)
            p.write_text(':020000040003F7\n:04E000000C9400017B\n:00000001FF\n')
            with self.assertRaises(ValueError):
                f.empaquetar(p)

    def test_trama_rdm(self):
        destino = f.uid('7ff0:66601337')
        p = f.trama_rdm(destino, 0x20, 0x1200, b'\0\x01\x01\0\0\0', 37)
        self.assertEqual(p[3:9], destino)
        self.assertEqual(p[15], 37)
        self.assertEqual(p[21:24], b'\x12\0\x06')
        self.assertEqual(sum(p[:-2]) & 0xFFFF, int.from_bytes(p[-2:], 'big'))
        with self.assertRaises(ValueError):
            f.uid('ffff:ffffffff')

    def test_identidad_enttec(self):
        controlador = f.uid_enttec(bytes.fromhex('11083502'))
        self.assertEqual(controlador.hex(), '454e02350811')
        caso = self

        class Interfaz(f.Enttec):
            def __init__(self):
                self.destino = f.uid('7ff0:66601337')
                self.uid_controlador = controlador
                self.transaccion = 0

            def enviar(self, etiqueta, p):
                caso.assertEqual(etiqueta, 7)
                caso.assertEqual(p[9:15], controlador)
                r = (b'\xcc\x01\x1a' + controlador + self.destino +
                     bytes((p[15], 0, 0, 0, 0, p[20] + 1)) + p[21:23] + b'\x02\x00\x01')
                self.respuesta = r + struct.pack('>H', sum(r) & 65535)

            def recibir(self, fin):
                return 5, b'\x00' + self.respuesta

        self.assertEqual(Interfaz().comando(0x20, 0xf0, b''), b'\x00\x01')

    def test_carga_controlador(self):
        archivo = self.paquete()
        caso = self

        class Transporte:
            sesion = 0
            iniciado = False
            confirmado = False
            recibido = b''

            def comando(self, clase, pid, p):
                if pid == 0x1200:
                    caso.assertEqual(f.crc16(p[:-2]), int.from_bytes(p[-2:], 'big'))
                    if not self.iniciado:
                        self.iniciado = True
                        return b'\x06\0\0\0\x01' + bytes(33)
                    self.sesion = p[0]
                    r = bytearray(38)
                    r[0], r[5], r[6], r[21] = 1, self.sesion, 1, 128
                    r[7:9] = b'\x01\0'
                    return bytes(r)
                if pid == 0x1201:
                    caso.assertEqual(p[0], self.sesion)
                    if clase == 0x30:
                        caso.assertEqual(f.crc16(p[:-2]), int.from_bytes(p[-2:], 'big'))
                        caso.assertEqual(int.from_bytes(p[1:5], 'big'), len(self.recibido))
                        self.recibido += p[5:-2]
                        return b'\x03\0\0\0\x01' + bytes(4)
                    return b'\x04' + bytes(4) + struct.pack('>I', len(self.recibido))
                if pid == 0x1202:
                    caso.assertEqual(p, bytes((self.sesion, 0, 0)))
                    self.confirmado = clase == 0x30
                    return bytes(7) + struct.pack('>H', f.crc16(archivo)) + f.uid('7ff0:66601337')
                caso.assertTrue(self.confirmado)
                return b'0.0.5 RDM prototipo'

        transporte = Transporte()
        with patch.object(f.time, 'sleep'), contextlib.redirect_stdout(io.StringIO()):
            f.cargar(transporte, archivo)
        self.assertEqual(transporte.recibido, archivo)
        self.assertTrue(transporte.confirmado)


if __name__ == '__main__':
    unittest.main()
