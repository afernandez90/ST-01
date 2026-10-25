#pragma once
#include <stdint.h>

// Motor sin memoria por píxel: apto para AVR y para pruebas en el ordenador.
namespace Efectos {
constexpr uint8_t PASOS_BARRA = 60;
struct Modo { uint8_t efecto, paso, pasos; };
// 0: fijo; 1: progreso manual; 2–25: las 24 animaciones.
inline Modo decodificar(uint8_t valor, bool barra) {
  if (!valor) return {0,0,1};
  if (barra && valor<=PASOS_BARRA) return {1,valor,PASOS_BARRA};
  const uint16_t inicio=barra ? PASOS_BARRA+1 : 1;
  const uint16_t disponibles=256-inicio;
  for(uint8_t clase=0;clase<24;++clase) {
    const uint16_t minimo=inicio+uint16_t(clase)*disponibles/24;
    const uint16_t fin=inicio+uint16_t(clase+1)*disponibles/24;
    if(valor<fin) return {uint8_t(clase+2),uint8_t(valor-minimo),uint8_t(fin-minimo)};
  }
  return {0,0,1};
}
inline uint16_t periodo(Modo m) {
  // Ciclo lento → rápido. Estrobo: 2–10 Hz; nunca más que el refresco de 20 Hz.
  const uint16_t lento=m.efecto==5 ? 500 : 6000;
  const uint16_t rapido=m.efecto==5 ? 100 : 1000;
  return lento-uint32_t(lento-rapido)*m.paso/(m.pasos-1);
}
inline uint8_t ruido(uint32_t n) {
  n^=n>>16; n*=0x7feb352dUL; n^=n>>15; n*=0x846ca68bUL; n^=n>>16;
  return uint8_t(n);
}
inline uint8_t onda(uint8_t fase) {
  const uint16_t t=fase<128 ? uint16_t(fase)*2 : uint16_t(255-fase)*2;
  return t*t/255;
}
inline uint16_t distancia(uint16_t a,uint16_t b){return a>b ? a-b : b-a;}
inline uint8_t intensidad(Modo m,uint16_t i,uint16_t n,uint32_t tiempo) {
  if(!n || i>=n)return 0;
  if(!m.efecto)return 255;
  if(m.efecto==1)return i<uint32_t(m.paso)*n/PASOS_BARRA ? 255 : 0;
  const uint16_t duracion=periodo(m);
  const uint32_t ciclo=tiempo/duracion;
  const uint16_t fase=uint32_t(tiempo%duracion)*1024/duracion;
  const uint16_t posicion=uint32_t(fase)*n/1024;
  const uint16_t ancho=n/10 ? n/10 : 1;
  const uint16_t centroDist=distancia(uint32_t(i)*2,n-1);
  switch(m.efecto) {
    case 2:return onda(fase/4); // Respiración.
    case 3: // Dos pulsos con pausa.
      if(fase<160)return onda(uint32_t(fase)*255/160);
      if(fase>=220 && fase<380)return uint16_t(onda(uint32_t(fase-220)*255/160))*3/4;
      return 0;
    case 4:return fase<512 ? 255 : 0;
    case 5:return fase<512 ? 255 : 0;
    case 6:return (i+n-posicion)%n<ancho ? 255 : 0;
    case 7:return (n-1-i+n-posicion)%n<ancho ? 255 : 0;
    case 8: case 9: {
      const uint16_t x=m.efecto==8 ? i : n-1-i;
      const uint16_t cola=(posicion+n-x)%n;
      return cola<ancho*3 ? 255-uint32_t(cola)*255/(ancho*3) : 0;
    }
    case 10: { // KITT: un punto y los dos vecinos al 60 %.
      const uint16_t p=uint32_t(fase<512 ? fase : 1023-fase)*(n-1)/511;
      const uint16_t d=distancia(i,p);
      return d==0 ? 255 : d==1 ? 153 : 0;
    }
    case 11: {
      const uint16_t cantidad=uint32_t(fase<512 ? fase : 1023-fase)*n/511;
      return i<cantidad ? 255 : 0;
    }
    case 12:return distancia(centroDist,uint32_t(fase)*n/1024)<ancho ? 255 : 0;
    case 13:return distancia(centroDist,uint32_t(1023-fase)*n/1024)<ancho ? 255 : 0;
    case 14:return distancia(i,posicion)<ancho || distancia(n-1-i,posicion)<ancho ? 255 : 0;
    case 15: case 16: {
      const uint16_t x=m.efecto==15 ? i : n-1-i;
      const uint16_t separacion=ancho*4;
      const uint16_t avance=uint32_t(fase)*separacion/1024;
      return (x+separacion-avance)%separacion<ancho ? 255 : 0;
    }
    case 17:return onda(uint8_t(uint32_t(i)*768/n-fase/4));
    case 18: { // Chispas con caída dentro de cada intervalo.
      const uint32_t tick=ciclo*16+fase/64;
      return ruido(tick*131+i*977UL)<24 ? 255-uint16_t(fase%64)*255/64 : 0;
    }
    case 19: { // Ruido interpolado por zonas, conservando el RGB elegido.
      const uint32_t t=ciclo*12+uint32_t(fase)*12/1024;
      const uint16_t f=(uint32_t(fase)*12%1024)*255/1024;
      const uint32_t zona=i/(ancho*2);
      return 64+(uint32_t(ruido(t+zona*977))*(255-f)+uint32_t(ruido(t+1+zona*977))*f)*191/65025;
    }
    case 20:return ruido(ciclo*977)<180 && ((fase<60) || (fase>110 && fase<160) || (fase>230 && fase<260)) ? 255 : 0;
    case 21:return ruido((ciclo*24+uint32_t(fase)*24/1024)*131+i/(ancho*2)*977UL)<100 ? 0 : 255;
    case 22: {
      if(fase>=930)return 255;
      const uint16_t acelerada=uint32_t(fase)*fase/1024;
      return ((i+n-uint32_t(acelerada)*n/128)%n)<ancho ? uint32_t(fase)*255/1024 : 0;
    }
    case 23: {
      const uint16_t frente=uint32_t(fase)*n/512;
      if(centroDist>frente)return 0;
      const uint16_t edad=uint32_t(frente-centroDist)*512/n;
      return edad<400 ? 255-uint32_t(edad)*255/400 : 0;
    }
    case 24:return ((i/(ancho*2))%2)==(fase/512) ? 255 : 0;
    case 25: {
      const uint16_t desplazada=fase+uint16_t(ruido(i/(ancho*2)*977UL))*4;
      const uint8_t amplitud=ruido((ciclo+desplazada/1024)*131+i/(ancho*2)*977UL);
      return uint16_t(onda(uint8_t((desplazada%1024)/4)))*amplitud/255;
    }
  }
  return 0;
}
struct Reloj {
  uint8_t anterior=0;
  uint32_t inicio=0;
  uint32_t tiempo(uint8_t modo,uint32_t ahora) {
    if(modo!=anterior){anterior=modo;inicio=ahora;}
    return ahora-inicio;
  }
};
}
