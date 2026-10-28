"""Verificar el respaldo/restauración EEPROM por dirección SPI de byte."""
import contextlib
import importlib.util
import io
from pathlib import Path
import unittest
from unittest.mock import patch

ruta = Path(__file__).resolve().parents[1] / 'herramientas' / 'eeprom_isp.py'
spec = importlib.util.spec_from_file_location('eeprom_isp', ruta)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class MemoriaSimulada(m.ArduinoISP):
    def __init__(self):
        self.memoria = bytearray([255] * 4096)
        self.escrituras = []
        self.fallar = False

    def spi(self, a, b, c, d):
        direccion = b * 256 + c
        if a == 0xa0:
            return self.memoria[direccion]
        if a == 0xc0:
            self.escrituras.append((direccion, d))
            if not self.fallar:
                self.memoria[direccion] = d
            return 0
        raise AssertionError(f'Comando inesperado: {a}')


class PruebasEEPROM(unittest.TestCase):
    def test_respaldo_direcciones_byte_y_sin_alias(self):
        isp = MemoriaSimulada()
        for direccion, valor in [(16, 1), (32, 2), (2048, 3), (4095, 4)]:
            isp.memoria[direccion] = valor
        with contextlib.redirect_stdout(io.StringIO()):
            datos = isp.respaldo()
        self.assertEqual(datos, isp.memoria)
        self.assertEqual(isp.escrituras, [])

    def test_restaurar_solo_diferencias_y_verificar(self):
        isp = MemoriaSimulada()
        esperado = bytearray(isp.memoria)
        esperado[32], esperado[2048], esperado[4095] = 0x52, 0x34, 0x12
        with patch.object(m.time, 'sleep'), contextlib.redirect_stdout(io.StringIO()):
            isp.restaurar(esperado)
        self.assertEqual(isp.memoria, esperado)
        self.assertEqual(isp.escrituras, [(32, 0x52), (2048, 0x34), (4095, 0x12)])

    def test_longitud_invalida_no_escribe(self):
        isp = MemoriaSimulada()
        with self.assertRaises(ValueError):
            isp.restaurar(b'\0' * 4095)
        self.assertEqual(isp.escrituras, [])

    def test_fallo_de_verificacion_detiene_restauracion(self):
        isp = MemoriaSimulada()
        isp.fallar = True
        with patch.object(m.time, 'sleep'), self.assertRaises(RuntimeError):
            isp.restaurar(bytes(4096))
        self.assertEqual(isp.escrituras, [(0, 0)])


if __name__ == '__main__':
    unittest.main()
