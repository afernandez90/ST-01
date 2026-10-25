#pragma once
#include <stdint.h>
struct CRGB {
  uint8_t r=0,g=0,b=0;
  CRGB()=default;
  CRGB(uint8_t rojo,uint8_t verde,uint8_t azul):r(rojo),g(verde),b(azul){}
};
struct WS2812B {}; struct WS2811 {};
constexpr int GRB=1, RGB=2;
inline void fill_solid(CRGB *p,int n,CRGB c){for(int i=0;i<n;++i)p[i]=c;}
struct TirasSimuladas {
  template<class T,uint8_t P,int O> void addLeds(CRGB*,int){}
  void setBrightness(uint8_t){}
  void show(){}
  void clear(bool){}
};
inline TirasSimuladas FastLED;
