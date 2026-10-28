// Ejecutar el mismo receptor USB del cargador con flash/EEPROM simuladas.
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#include <iostream>
#include "../arduino/controlador/ftc.h"
uint8_t UCSR0A=0;
#define U2X0 1
#define TXC0 6
#define _BV(n) (1u<<(n))
#define GET_LOCK_BITS 0
#define GET_EXTENDED_FUSE_BITS 1
#define GET_LOW_FUSE_BITS 2
#define GET_HIGH_FUSE_BITS 3
uint8_t boot_lock_fuse_bits_get(int n){return n==GET_LOCK_BITS?0xef:0xff;}
uint8_t eeprom[4096]={};
uint8_t eeprom_read_byte(const uint8_t *p){auto i=reinterpret_cast<uintptr_t>(p);assert(i<4096);return eeprom[i];}
void eeprom_update_byte(uint8_t *p,uint8_t v){auto i=reinterpret_cast<uintptr_t>(p);assert(i<4096);eeprom[i]=v;}
struct Memoria {
 uint8_t flash[0x40000]={};unsigned escrituras=0;
 uint8_t leer(uint32_t p){assert(p<sizeof(flash));return flash[p];}
 bool escribir(uint32_t p,const uint8_t *d){assert(p+256<=Ftc::LIMITE_FLASH);memcpy(flash+p,d,256);++escrituras;return true;}
 void invalidar(){eeprom[Ftc::EEPROM_ESTADO]=Ftc::PENDIENTE;}
 void validar(){eeprom[Ftc::EEPROM_ESTADO]=Ftc::VALIDA;}
} memoria;
unsigned saltos=0;
void saltarAplicacion(){++saltos;}
std::vector<uint8_t> tx;
void ponerUart0(uint8_t b){tx.push_back(b);}
#include "../arduino/arranque_rdm/usb.h"
std::vector<uint8_t> enviar(std::vector<uint8_t> datos,bool valido=true){
 tx.clear();std::vector<uint8_t> trama={0x1b,7,uint8_t(datos.size()>>8),uint8_t(datos.size()),0x0e};
 trama.insert(trama.end(),datos.begin(),datos.end());uint8_t suma=0;for(auto b:trama)suma^=b;
 trama.push_back(valido?suma:suma^1);for(auto b:trama)recibirUsb(b);
 if(!valido){assert(tx.empty());return {};}
 assert(tx.size()>=8 && tx[0]==0x1b && tx[1]==7 && tx[4]==0x0e);
 assert(tx.size()==size_t((tx[2]<<8)|tx[3])+6);
 suma=0;for(auto b:tx)suma^=b;assert(suma==0);
 return std::vector<uint8_t>(tx.begin()+5,tx.end()-1);
}
void direccion(uint32_t palabra){assert(enviar({6,uint8_t(palabra>>24),uint8_t(palabra>>16),uint8_t(palabra>>8),uint8_t(palabra)})==std::vector<uint8_t>({6,0}));}
int main(){
 // Trama real observada en avrdude: READ_FLASH_ISP, 256 bytes, comando SPI 0x20.
 for(unsigned i=0;i<256;++i)memoria.flash[i]=uint8_t(i);
 direccion(0x80000000);auto r=enviar({0x14,1,0,0x20});
 assert(r.size()==259 && r[0]==0x14 && r[1]==0 && r.back()==0);
 for(unsigned i=0;i<256;++i)assert(r[i+2]==i);
 assert(memoria.escrituras==0 && eeprom[0]==0);
 // Direcciones extendidas por encima de 128 KiB.
 memoria.flash[0x20000]=0xab;direccion(0x80010000);
 assert(enviar({0x14,0,1,0x20})==std::vector<uint8_t>({0x14,0,0xab,0}));
 // PROGRAM_EEPROM_ISP es 0x15 y no debe tocar flash.
 direccion(16);r=enviar({0x15,0,2,0,0,0,0,0,0,0,0x12,0x34});
 assert(r==std::vector<uint8_t>({0x15,0}) && eeprom[16]==0x12 && eeprom[17]==0x34 && memoria.escrituras==0);
 direccion(16);assert(enviar({0x16,0,2,0xa0})==std::vector<uint8_t>({0x16,0,0x12,0x34,0}));
 // Programar una página, verificarla y no permitir escrituras en el cargador.
 direccion(0);std::vector<uint8_t> pagina={0x13,1,0,0,0,0,0,0,0,0};pagina.resize(266,0x5a);
 assert(enviar(pagina)==std::vector<uint8_t>({0x13,0}) && memoria.escrituras==1);
 direccion(0);r=enviar({0x14,1,0,0x20});for(unsigned i=0;i<256;++i)assert(r[i+2]==0x5a);
 direccion(Ftc::LIMITE_FLASH/2);assert(enviar(pagina)==std::vector<uint8_t>({0x13,0xc0}));assert(memoria.escrituras==1);
 // Límites de lectura y EEPROM, y checksum de trama inválido.
 direccion(0);assert(enviar({0x14,1,1,0x20})==std::vector<uint8_t>({0x14,0xc0}));
 direccion(4095);assert(enviar({0x16,0,2,0xa0})==std::vector<uint8_t>({0x16,0xc0}));
 direccion(0);enviar(pagina,false);assert(memoria.escrituras==1);
 std::cout<<"USB STK500v2: flash, EEPROM, direcciones extendidas, limites y checksum correctos.\n";
}
