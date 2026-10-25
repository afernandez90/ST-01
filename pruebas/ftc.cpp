// Integridad, estados y límites de la actualización, sin escribir una placa real.
#include <cassert>
#include <vector>
#include <iostream>
#include "../arduino/pruebas/recepcion_dmx/ftc.h"
struct Memoria {
  std::vector<uint8_t> flash=std::vector<uint8_t>(Ftc::LIMITE_FLASH,0xff);
  bool valida=true;unsigned escrituras=0;
  void invalidar(){valida=false;}
  void validar(){valida=true;}
  uint8_t leer(uint32_t p){return flash.at(p);}
  bool escribir(uint32_t p,const uint8_t *v){
    assert(p%256==0 && p+256<=Ftc::LIMITE_FLASH);
    std::copy(v,v+256,flash.begin()+p);++escrituras;return true;
  }
};
using Bytes=std::vector<uint8_t>;
Bytes archivo(){
  Bytes p(16+513);memcpy(p.data(),"ST01",4);p[4]=1;p[5]=0x25;Ftc::poner16(p.data()+6,1);
  Ftc::poner32(p.data()+8,p.size()-16);uint32_t crc=0xffffffff;
  for(unsigned i=16;i<p.size();++i){p[i]=i&255;crc=Ftc::crc32Byte(crc,p[i]);}
  Ftc::poner32(p.data()+12,crc^0xffffffff);return p;
}
Bytes iniciar(const Bytes &archivo,bool prueba=false){
  Bytes p(30);p[0]=7;p[1]=1;Ftc::poner16(p.data()+2,0x100);Ftc::poner16(p.data()+4,prueba?1:0);
  Ftc::poner32(p.data()+6,archivo.size());memcpy(p.data()+10,archivo.data(),16);
  Ftc::poner16(p.data()+26,Ftc::crc16(archivo.data(),archivo.size()));
  Ftc::poner16(p.data()+28,Ftc::crc16(p.data(),28));return p;
}
Bytes datos(const Bytes &archivo,uint32_t offset){
  const auto tamano=std::min<uint32_t>(128,archivo.size()-offset);Bytes p(7+tamano);p[0]=7;
  Ftc::poner32(p.data()+1,offset);memcpy(p.data()+5,archivo.data()+offset,tamano);
  Ftc::poner16(p.data()+p.size()-2,Ftc::crc16(p.data(),p.size()-2));return p;
}
int main(){
  assert(Ftc::crc16((const uint8_t*)"123456789",9)==0x4b37);
  Memoria m;Ftc::Receptor<Memoria> r(m,true,true);uint8_t salida[231],n;
  auto enviar=[&](uint16_t pid,bool w,const Bytes &p){return r.comando(pid,w,p.data(),p.size(),salida,n);};
  const Bytes imagen=archivo();
  assert(enviar(0x1200,false,{0,0,1,0,0,0})==0xffff && n==38 && salida[0]==0);
  assert(Ftc::leer16(salida+7)==0x100 && salida[21]==128);
  assert(enviar(0x1200,false,{0,0,0,56,0,0})==0xffff && salida[0]==0x16);
  auto comienzo=iniciar(imagen);comienzo.back()^=1;
  assert(enviar(0x1200,true,comienzo)==0xffff && salida[0]==11 && m.valida);
  comienzo=iniciar(imagen);
  assert(enviar(0x1200,true,comienzo)==0xffff && salida[0]==1 && m.valida);
  r.ejecutarPendiente();assert(!m.valida && r.estado==2);
  auto bloque=datos(imagen,128);
  assert(enviar(0x1201,true,bloque)==0xffff && salida[0]==0x14 && r.recibidos==0);
  bloque=datos(imagen,0);bloque.back()^=1;
  assert(enviar(0x1201,true,bloque)==0xffff && salida[0]==11 && r.recibidos==0);
  for(uint32_t offset=0;offset<imagen.size();offset+=128){
    bloque=datos(imagen,offset);
    assert(enviar(0x1201,true,bloque)==0xffff && salida[0]==3);
    assert(Ftc::leer32(salida+5)==offset);
    r.ejecutarPendiente();const auto escritos=m.escrituras;
    assert(enviar(0x1201,true,bloque)==0xffff && (salida[0]==0 || salida[0]==4));
    assert(m.escrituras==escritos); // Repetir no reprograma ni duplica el CRC.
  }
  assert(r.recibidos==imagen.size());
  for(unsigned i=0;i<imagen.size()+2;++i)r.ejecutarPendiente();
  assert(r.validado && !m.valida && m.escrituras==3);
  assert(enviar(0x1202,false,{7,0,0})==0xffff && salida[0]==0 && n==15);
  assert(enviar(0x1202,true,{8,0,0})==0xffff && salida[0]==7 && !m.valida);
  assert(enviar(0x1202,true,{7,0,1})==0xffff && salida[0]==5 && !m.valida);
  assert(enviar(0x1202,true,{7,0,0})==0xffff && salida[0]==0 && !m.valida);
  r.ejecutarPendiente();assert(m.valida && r.reiniciar);
  assert(std::equal(imagen.begin()+16,imagen.end(),m.flash.begin()));
  // Corte de alimentación/cancelación: la marca de recuperación sigue pendiente.
  assert(enviar(0x1200,true,iniciar(imagen))==0xffff);r.reiniciar=false;r.ejecutarPendiente();
  assert(!m.valida);assert(enviar(0x1203,true,{255})==0xffff && salida[0]==0 && r.sesion==0);
  Ftc::Receptor<Memoria> trasCorte(m,true,true);assert(!m.valida && !trasCorte.reiniciar);
  // Testmode no modifica la memoria ni la marca de aplicación.
  m.valida=true;const auto antes=m.escrituras;
  assert(enviar(0x1200,true,iniciar(imagen,true))==0xffff);r.ejecutarPendiente();
  for(uint32_t offset=0;offset<imagen.size();offset+=128){enviar(0x1201,true,datos(imagen,offset));r.ejecutarPendiente();}
  r.ejecutarPendiente();assert(r.validado && m.valida && m.escrituras==antes);
  assert(enviar(0x1202,true,{7,0,1})==0xffff && salida[0]==0 && !r.activo);
  // Un CRC de archivo erróneo impide el compromiso aunque el CRC de paquete sea válido.
  comienzo=iniciar(imagen);comienzo[26]^=1;Ftc::poner16(comienzo.data()+28,Ftc::crc16(comienzo.data(),28));
  enviar(0x1200,true,comienzo);r.ejecutarPendiente();
  for(uint32_t offset=0;offset<imagen.size();offset+=128){enviar(0x1201,true,datos(imagen,offset));r.ejecutarPendiente();}
  r.ejecutarPendiente();assert(enviar(0x1202,true,{7,0,0})==0xffff && salida[0]==12 && !m.valida);
  // La aplicación solo solicita el reinicio cuando existe un cargador compatible.
  Ftc::Receptor<Memoria> aplicacion(m,false,false);auto peticion=iniciar(imagen);
  assert(aplicacion.comando(0x1200,true,peticion.data(),peticion.size(),salida,n)==0xffff && salida[0]==0x12);
  aplicacion.disponible=true;
  assert(aplicacion.comando(0x1200,true,peticion.data(),peticion.size(),salida,n)==0xffff && salida[0]==6 && aplicacion.reiniciar);
  std::cout<<"FTC: CRC, estados, reintentos, proteccion, cancelacion, recuperacion y testmode correctos.\n";
}
