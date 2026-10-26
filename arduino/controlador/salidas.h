#pragma once
#include <stdint.h>
#include <string.h>

constexpr uint8_t CANALES_ST01 = 19;
// Fotograma lógico de todas las salidas, incluido el piloto de estado independiente.
struct EstadoSalidas {
  uint8_t canales[CANALES_ST01];
  bool pilotoEstado;
};

inline EstadoSalidas calcularSalidas(const uint8_t *dmx, bool activo,
                                    bool identificar, unsigned long ahora) {
  EstadoSalidas estado = {};
  if (identificar) {
    const bool encendido = (ahora / 250) % 2;
    if (encendido) {
      // 19 representa el 8 con punto: todos los segmentos encendidos.
      for (uint8_t i = 0; i < 4; ++i) estado.canales[i] = 19;
      for (uint8_t i = 4; i < CANALES_ST01; ++i) estado.canales[i] = 255;
      estado.canales[7] = estado.canales[18] = 0;  // RGB fijo en ambas tiras.
    }
    estado.pilotoEstado = encendido;
  } else if (activo) {
    memcpy(estado.canales, dmx, CANALES_ST01);
    estado.pilotoEstado = true;
  }
  return estado;
}
