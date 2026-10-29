#include <cassert>
#include <cstdint>
#include <iostream>
#include "../arduino/controlador/digitos.h"
uint8_t segmentos(const char *letras) {
  uint8_t mascara=0;
  while(*letras)mascara|=uint8_t(1)<<(*letras++-'a');
  return mascara;
}
int main() {
  assert(mascaraDigito(0)==0);
  const char *numeros[]={"abcdef","bc","abdeg","abcdg","bcfg","acdfg","acdefg","abc","abcdefg","abcdfg"};
  for(unsigned i=0;i<10;++i) {
    assert(mascaraDigito(i ? i : 10)==segmentos(numeros[i]));
    assert(mascaraDigito(i ? 10+i : 20)==(segmentos(numeros[i])|0x80));
  }
  assert(mascaraDigito(11)==0x86); // 1.
  assert(mascaraDigito(18)==0xff); // 8., todos los segmentos
  assert(mascaraDigito(19)==0xef); // 9.
  assert(mascaraDigito(20)==0xbf); // 0.
  for(unsigned i=0;i<7;++i) {
    assert(mascaraDigito(21+i)==(1U<<i));
    assert(mascaraDigito(28+i)==((1U<<i)|0x80));
  }
  for(unsigned i=35;i<=86;++i)assert(mascaraDigito(i)>0 && !(mascaraDigito(i)&0x80));
  assert(mascaraDigito(87)==0x80); // Solo DP, ningún A–G.
  for(unsigned i=88;i<256;++i)assert(mascaraDigito(i)==0);
  assert(mascaraDigito(35)==segmentos("abcefg")); // A
  assert(mascaraDigito(37)==segmentos("adef")); // C
  assert(mascaraDigito(63)==segmentos("deg")); // c
  assert(mascaraDigito(49)==segmentos("abcdef")); // O
  assert(mascaraDigito(75)==segmentos("cdeg")); // o
  assert(mascaraDigito(60)==segmentos("abdeg")); // Z
  assert(mascaraDigito(86)==segmentos("abdeg")); // z, misma aproximación
  assert(mascaraDigito(35)!=mascaraDigito(61)); // A y a
  assert(mascaraDigito(42)!=mascaraDigito(68)); // H y h
  std::cout<<"Digitos: codificacion anterior, 52 letras, glifos y valores sin asignar correctos.\n";
}
