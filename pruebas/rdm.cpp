// Pruebas de tramas RDM sin placa; ejecutar con pruebas/rdm.sh.
#include <assert.h>
#include <stdio.h>
#include <vector>
#include "../arduino/controlador/rdm.h"
using Trama = std::vector<uint8_t>;
RespondedorRdm equipo;
const uint8_t controlador[] = {0x12, 0x34, 0, 0, 0, 2};
void sellar(Trama &p) {
  uint16_t suma = 0;
  for (unsigned i = 0; i < p.size() - 2; ++i) suma += p[i];
  RespondedorRdm::escribir16(p.data() + p.size() - 2, suma);
}
Trama peticion(uint8_t clase, uint16_t pid, Trama datos = {}, bool difusion = false) {
  Trama p(26 + datos.size());
  p[0] = 0xcc; p[1] = 1; p[2] = p.size() - 2;
  memcpy(p.data() + 3, equipo.uid, 6);
  if (difusion) memset(p.data() + 3, 0xff, 6);
  memcpy(p.data() + 9, controlador, 6);
  p[15] = 73; p[16] = 1; p[20] = clase;
  RespondedorRdm::escribir16(p.data() + 21, pid);
  p[23] = datos.size();
  memcpy(p.data() + 24, datos.data(), datos.size());
  sellar(p);
  return p;
}
bool enviar(const Trama &p) { return equipo.procesar(p.data(), p.size()); }
void validarRespuesta(uint8_t clase, uint16_t pid, uint8_t tipo = 0) {
  const auto *p = equipo.salida;
  assert(!equipo.descubrimiento);
  assert(p[0] == 0xcc && p[1] == 1 && p[15] == 73);
  assert(!memcmp(p + 3, controlador, 6) && !memcmp(p + 9, equipo.uid, 6));
  assert(p[16] == tipo && p[17] == 0 && p[20] == clase + 1);
  assert(RespondedorRdm::leer16(p + 21) == pid);
  assert(p[23] + 26 == equipo.longitudSalida);
  uint16_t suma = 0;
  for (unsigned i = 0; i < equipo.longitudSalida - 2; ++i) suma += p[i];
  assert(suma == RespondedorRdm::leer16(p + equipo.longitudSalida - 2));
}
void nack(uint8_t clase, uint16_t pid, Trama datos, uint16_t motivo) {
  assert(enviar(peticion(clase, pid, datos)));
  validarRespuesta(clase, pid, 2);
  assert(equipo.salida[23] == 2 && RespondedorRdm::leer16(equipo.salida + 24) == motivo);
}
int main() {
  assert(enviar(peticion(0x20, 0x60)));
  validarRespuesta(0x20, 0x60);
  assert(equipo.salida[23] == 19);
  assert(RespondedorRdm::leer16(equipo.salida + 34) == 19);
  assert(RespondedorRdm::leer16(equipo.salida + 38) == 1);
  for (auto pid : {0x50, 0x80, 0x81, 0x82, 0xc0, 0xe0, 0xf0, 0x1000}) {
    assert(enviar(peticion(0x20, pid)));
    validarRespuesta(0x20, pid);
  }
  assert(enviar(peticion(0x20, 0xe1, {1})));
  validarRespuesta(0x20, 0xe1);
  assert(equipo.salida[23] == 17 && equipo.salida[24] == 1);
  for (int direccion : {1, 494, 200}) {
    assert(enviar(peticion(0x30, 0xf0, {uint8_t(direccion >> 8), uint8_t(direccion)})));
    validarRespuesta(0x30, 0xf0);
    assert(equipo.direccion == direccion && equipo.guardar);
    assert(enviar(peticion(0x20, 0xf0)));
    assert(RespondedorRdm::leer16(equipo.salida + 24) == direccion);
  }
  nack(0x30, 0xf0, {0, 0}, 6);
  nack(0x30, 0xf0, {1, 239}, 6);  // 495 no deja espacio para 19 canales.
  nack(0x30, 0xf0, {1}, 1);
  nack(0x20, 0x60, {0}, 1);
  nack(0x30, 0x60, {}, 5);
  nack(0x20, 0x7777, {}, 0);
  nack(0x30, 0xe0, {2}, 6);
  nack(0x20, 0xe1, {2}, 6);
  nack(0x30, 0x1000, {2}, 6);
  assert(equipo.direccion == 200);
  auto p = peticion(0x20, 0x60); p[19] = 1; sellar(p);
  assert(enviar(p)); validarRespuesta(0x20, 0x60, 2);
  assert(RespondedorRdm::leer16(equipo.salida + 24) == 9);
  assert(enviar(peticion(0x30, 0x1000, {1})) && equipo.identificar);
  assert(enviar(peticion(0x30, 0x1000, {0})) && !equipo.identificar);
  Trama etiqueta(32, 'X');
  assert(enviar(peticion(0x30, 0x82, etiqueta)));
  assert(enviar(peticion(0x20, 0x82)) && equipo.salida[23] == 32);
  assert(!memcmp(equipo.salida + 24, etiqueta.data(), 32));
  nack(0x30, 0x82, Trama(33, 'X'), 1);
  // Difusión: aplicar SET sin responder, ignorar GET y otros fabricantes.
  assert(!enviar(peticion(0x30, 0xf0, {0, 42}, true)) && equipo.direccion == 42);
  assert(!enviar(peticion(0x20, 0xf0, {}, true)));
  p = peticion(0x30, 0xf0, {0, 43}, true);
  p[3] = equipo.uid[0]; p[4] = equipo.uid[1]; sellar(p);
  assert(!enviar(p) && equipo.direccion == 43);
  p[4] ^= 1; sellar(p);
  assert(!enviar(p) && equipo.direccion == 43);
  // Descubrimiento: extremos inclusivos y suma de los bytes codificados.
  Trama limites(equipo.uid, equipo.uid + 6);
  limites.insert(limites.end(), equipo.uid, equipo.uid + 6);
  assert(enviar(peticion(0x10, 1, limites, true)) && equipo.descubrimiento);
  assert(equipo.longitudSalida == 24 && equipo.salida[7] == 0xaa);
  uint16_t suma = 0;
  for (int i = 8; i < 20; ++i) suma += equipo.salida[i];
  uint16_t codificada = ((equipo.salida[20] & equipo.salida[21]) << 8) |
                       (equipo.salida[22] & equipo.salida[23]);
  assert(suma == codificada);
  limites[11] = 0;
  assert(!enviar(peticion(0x10, 1, limites, true)));
  assert(enviar(peticion(0x10, 2)) && equipo.silenciado);
  validarRespuesta(0x10, 2);
  assert(!enviar(peticion(0x10, 1, Trama(12, 0xff), true)));
  assert(!enviar(peticion(0x10, 3, {}, true)) && !equipo.silenciado);
  // Tramas truncadas, suma errónea, destino ajeno y longitudes incoherentes.
  p = peticion(0x30, 0xf0, {0, 99});
  p.back() ^= 1; assert(!enviar(p) && equipo.direccion == 43);
  p = peticion(0x20, 0x60); p[8] ^= 1; sellar(p); assert(!enviar(p));
  p = peticion(0x20, 0x60); p[23] = 1; sellar(p); assert(!enviar(p));
  for (unsigned n = 0; n < 26; ++n) assert(!equipo.procesar(p.data(), n));
  // Máximo tamaño RDM: 255 bytes de mensaje y dos bytes de suma.
  assert(enviar(peticion(0x20, 0x7777, Trama(231, 0xaa))));
  validarRespuesta(0x20, 0x7777, 2);
  p = peticion(0x20, 0x60); p[20] = 0x21; sellar(p); assert(!enviar(p));
  p = peticion(0x30, 0xf0, {0, 44}); p[18] = p[19] = 0xff; sellar(p);
  assert(enviar(p) && equipo.direccion == 44);  // SET a todos los subdispositivos.
  // Datos arbitrarios con sanitizadores: comprobar todos los tamaños aceptables.
  uint32_t aleatorio = 12345;
  for (unsigned intento = 0; intento < 20000; ++intento) {
    Trama basura(intento % 300);
    for (auto &b : basura) { aleatorio = aleatorio*1664525u + 1013904223u; b = aleatorio >> 24; }
    enviar(basura);
  }
  puts("RDM: descubrimiento, GET/SET, difusion, errores y limites correctos.");
}
