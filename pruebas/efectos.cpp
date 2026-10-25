#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include "../arduino/pruebas/recepcion_dmx/efectos.h"
using namespace Efectos;
int main() {
  for(bool barra:{false,true}) {
    unsigned clases[26]={};
    for(unsigned valor=0;valor<256;++valor) {
      const Modo m=decodificar(valor,barra);
      assert(m.efecto<=25);++clases[m.efecto];
      if(m.efecto>=2) {
        assert(m.paso<m.pasos && m.pasos>=8 && m.pasos<=11);
        const auto lento=periodo({m.efecto,0,m.pasos});
        const auto rapido=periodo({m.efecto,uint8_t(m.pasos-1),m.pasos});
        assert(lento==(m.efecto==5 ? 500 : 6000));
        assert(rapido==(m.efecto==5 ? 100 : 1000));
        assert(periodo(m)<=lento && periodo(m)>=rapido);
      }
      // Incluye longitudes mínimas, pares, impares y las dos tiras previstas.
      for(uint16_t n:{1,2,3,60,299,300}) {
        for(uint32_t tiempo:{0U,50U,250U,500U,999U,1000U,5999U,6000U,0xffffffffU}) {
          for(uint16_t i=0;i<n;++i) {
            const auto brillo=intensidad(m,i,n,tiempo);
            if(!valor)assert(brillo==255);
            if(barra && valor>=1 && valor<=60)
              assert(brillo==(i<uint32_t(valor)*n/60 ? 255 : 0));
          }
          assert(intensidad(m,0,0,tiempo)==0);
          assert(intensidad(m,n,n,tiempo)==0);
        }
      }
    }
    assert(clases[0]==1 && clases[1]==(barra ? 60U : 0U));
    for(unsigned i=2;i<26;++i)assert(clases[i]>=8 && clases[i]<=11);
  }
  for(uint8_t valor=1;valor<=60;++valor) {
    unsigned encendidos=0;
    for(unsigned i=0;i<60;++i)encendidos+=intensidad(decodificar(valor,true),i,60,0)>0;
    assert(encendidos==valor);
  }
  const Modo kitt={10,0,8};
  for(unsigned i=0;i<60;++i) {
    assert(intensidad(kitt,i,60,0)==(i==0 ? 255 : i==1 ? 153 : 0));
    assert(intensidad(kitt,i,60,3000)==(i==59 ? 255 : i==58 ? 153 : 0));
  }
  for(uint32_t t=0;t<6000;t+=25) {
    unsigned cabeza=0;
    for(unsigned i=0;i<60;++i)cabeza+=intensidad(kitt,i,60,t)==255;
    assert(cabeza==1);
  }
  // Cada animación cambia con el tiempo y llega a emitir luz.
  for(uint8_t efecto=2;efecto<26;++efecto) {
    bool cambia=false,ilumina=false;
    const Modo m={efecto,0,8};
    for(uint32_t t=0;t<18000;t+=50)
      for(uint16_t i=0;i<60;++i) {
        const auto b=intensidad(m,i,60,t);
        ilumina|=b>0;cambia|=b!=intensidad(m,i,60,0);
      }
    assert(cambia && ilumina);
  }
  Reloj r;
  assert(r.tiempo(100,1000)==0);
  assert(r.tiempo(100,1500)==500);
  assert(r.tiempo(101,1500)==0);
  r.inicio=std::numeric_limits<uint32_t>::max()-49;r.anterior=101;
  assert(r.tiempo(101,50)==100);
  std::cout<<"Efectos: 256 modos, velocidades, progreso exacto, KITT, animaciones y reloj correctos.\n";
}
