#pragma once
#include <cstring>
struct Memoria {
  uint8_t datos[4096] = {};
  template<class T> void get(int p,T &v){memcpy(&v, datos+p, sizeof(T));}
  template<class T> void put(int p,const T &v){memcpy(datos+p, &v, sizeof(T));}
  uint8_t read(int p){return datos[p];}
  void update(int p,uint8_t v){datos[p]=v;}
};
inline Memoria EEPROM;
