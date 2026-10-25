#pragma once
#include <stdint.h>
#include <string.h>
#include "identidad.h"

// Implementación propia de FTC 1.00 (E1.37-4-2026). No incorpora el documento licenciado.
namespace Ftc {
constexpr uint32_t LIMITE_FLASH=0x3e000UL, DIRECCION_FIRMA=0x3fff0UL;
constexpr uint16_t EEPROM_ESTADO=96;
constexpr uint8_t PENDIENTE=0x5a, SOLICITUD=0x5b, VALIDA=0xa5;
inline uint16_t leer16(const uint8_t *p){return (uint16_t(p[0])<<8)|p[1];}
inline uint32_t leer32(const uint8_t *p){return (uint32_t(leer16(p))<<16)|leer16(p+2);}
inline void poner16(uint8_t *p,uint16_t n){p[0]=n>>8;p[1]=n;}
inline void poner32(uint8_t *p,uint32_t n){poner16(p,n>>16);poner16(p+2,n);}
inline uint16_t crcByte(uint16_t crc,uint8_t b){
  crc^=b;for(uint8_t i=0;i<8;++i)crc=(crc>>1)^((crc&1)?0xa001:0);return crc;
}
inline uint16_t crc16(const uint8_t *p,uint16_t n){
  uint16_t crc=0xffff;while(n--)crc=crcByte(crc,*p++);return crc;
}
inline uint32_t crc32Byte(uint32_t crc,uint8_t b){
  crc^=b;for(uint8_t i=0;i<8;++i)crc=(crc>>1)^((crc&1)?0xedb88320UL:0);return crc;
}
// Archivo ST01: cabecera de 16 bytes + imagen binaria. Objetivo Mega2560/modelo 1.
inline bool cabeceraValida(const uint8_t *p,uint32_t total){
  return !memcmp(p,"ST01",4) && p[4]==1 && p[5]==0x25 && leer16(p+6)==1 &&
         total>=20 && total<=LIMITE_FLASH+16 && leer32(p+8)==total-16;
}

template<class Memoria> class Receptor {
 public:
  Memoria &memoria;
  bool esArranque, disponible;
  volatile bool reiniciar=false;
  bool confirmar=false, prueba=false, activo=false, validado=false;
  uint8_t sesion=0, estado=0, fallo=0;
  uint32_t total=0, recibidos=0, desplazamiento=0;
  uint16_t crcArchivo=0xffff, crcEsperado=0;
  uint32_t crcImagen=0xffffffffUL,crcFlash=0xffffffffUL,posicionValidacion=0;
  uint8_t cabecera[16]={}, pagina[256], cola[128], cantidad=0;
  uint32_t ultimoOffset=0;
  uint8_t ultimo[128], longitudUltimo=0;
  uint8_t uid[6]=ST01_UID_BYTES;
  explicit Receptor(Memoria &m,bool arranque,bool instalado):memoria(m),esArranque(arranque),disponible(instalado){memset(pagina,0xff,256);}
  uint32_t capacidades() const {
    return (disponible?1UL:0UL)|0x0cUL|0x00800600UL|(esArranque?0UL:0x80UL);
  }
  void respuesta(uint8_t *p,uint8_t codigo,uint32_t dato=0){p[0]=codigo;poner32(p+1,dato);}
  void inicioRespuesta(uint8_t *p,uint8_t &n){
    n=38;memset(p,0,n);p[5]=sesion;p[6]=1;poner16(p+7,0x0100);
    poner32(p+9,activo?total:LIMITE_FLASH+16);poner32(p+13,capacidades());
    poner32(p+17,recibidos);p[21]=128;poner32(p+22,20);poner16(p+26,20);
    poner16(p+34,15000); // Validación máxima declarada, en milisegundos.
  }
  uint16_t comando(uint16_t pid,bool escritura,const uint8_t *p,uint8_t n,uint8_t *s,uint8_t &l){
    l=0;
    if(pid<0x1200 || pid>0x1203)return 0;
    if(pid==0x1200){
      if(n!=(escritura?30:6))return 1;
      inicioRespuesta(s,l);
      if(escritura)s[5]=p[0];
      if(leer16(p+2)!=0x0100){respuesta(s,0x16,0x00010000);return 0xffff;}
      if(p[1]!=0 && p[1]!=1){respuesta(s,8,(uint32_t(p[1])<<16)|1);return 0xffff;}
      if(!escritura){
        if(p[0])respuesta(s,7,uint32_t(p[0])<<16);
        return 0xffff;
      }
      if(!p[0] || p[0]==255){respuesta(s,7,(uint32_t(p[0])<<16)|sesion);return 0xffff;}
      const uint16_t calculado=crc16(p,28);
      if(calculado!=leer16(p+28)){respuesta(s,11,(uint32_t(leer16(p+28))<<16)|calculado);return 0xffff;}
      if(leer16(p+4)&2){respuesta(s,0x13);return 0xffff;}
      if(leer16(p+4)&~uint16_t(1)){respuesta(s,5);return 0xffff;}
      if(!cabeceraValida(p+10,leer32(p+6))){respuesta(s,9);return 0xffff;}
      if(!disponible){respuesta(s,0x12);return 0xffff;}
      if(!esArranque){reiniciar=true;respuesta(s,6,1500);return 0xffff;}
      if(estado==1 || estado==3 || estado==4){respuesta(s,3,20);return 0xffff;}
      sesion=p[0];prueba=leer16(p+4)&1;total=leer32(p+6);crcEsperado=leer16(p+26);
      recibidos=0;crcArchivo=0xffff;crcImagen=crcFlash=0xffffffffUL;posicionValidacion=0;fallo=0;longitudUltimo=0;
      memcpy(cabecera,p+10,16);memset(pagina,0xff,256);
      activo=true;validado=false;confirmar=false;reiniciar=false;estado=1;
      inicioRespuesta(s,l);respuesta(s,1);return 0xffff;
    }
    if(pid==0x1203){
      if(!escritura)return 5;if(n!=1)return 1;l=5;
      if(p[0]!=255 && p[0]!=sesion){respuesta(s,7,(uint32_t(p[0])<<16)|sesion);return 0xffff;}
      activo=false;sesion=0;estado=0;cantidad=0;reiniciar=true;confirmar=false;
      respuesta(s,0);return 0xffff;
    }
    if(pid==0x1201){
      if(n<(escritura?7:1) || (!escritura && n!=1) || (escritura && n>135))return 1;
      l=9;memset(s,0,l);poner32(s+5,recibidos);
      if(!activo){respuesta(s,5);return 0xffff;}
      if(p[0]!=sesion){respuesta(s,7,(uint32_t(p[0])<<16)|sesion);return 0xffff;}
      if(!escritura){
        respuesta(s,fallo?fallo:(estado==1 || estado==3)?3:(recibidos==total)?4:0,
                  (estado==1 || estado==3)?20:0);return 0xffff;
      }
      const uint16_t calculado=crc16(p,n-2);
      if(calculado!=leer16(p+n-2)){respuesta(s,11,(uint32_t(leer16(p+n-2))<<16)|calculado);return 0xffff;}
      const uint32_t offset=leer32(p+1);const uint8_t tamano=n-7;
      if(longitudUltimo && offset==ultimoOffset && tamano==longitudUltimo && !memcmp(p+5,ultimo,tamano)){
        respuesta(s,recibidos==total?4:0);return 0xffff;
      }
      if(estado!=2){respuesta(s,3,20);return 0xffff;}
      if(offset!=recibidos){respuesta(s,0x14,recibidos);return 0xffff;}
      if(!tamano || tamano>total-recibidos || (tamano!=128 && tamano!=total-recibidos)){
        respuesta(s,0x15,total);return 0xffff;
      }
      memcpy(cola,p+5,tamano);cantidad=tamano;desplazamiento=offset;estado=3;
      respuesta(s,3,20);return 0xffff;
    }
    if(n!=3)return 1;
    l=15;memset(s,0,l);poner16(s+5,leer16(p+1));poner16(s+7,crcArchivo);memcpy(s+9,uid,6);
    if(!activo){respuesta(s,5);return 0xffff;}
    if(p[0]!=sesion){respuesta(s,7,(uint32_t(p[0])<<16)|sesion);return 0xffff;}
    if(leer16(p+1)!=(prueba?1:0)){respuesta(s,5);return 0xffff;}
    if(fallo){respuesta(s,fallo,fallo==12?((uint32_t(crcEsperado)<<16)|crcArchivo):0);return 0xffff;}
    if(estado==4){respuesta(s,3,15000);return 0xffff;}
    if(!validado){respuesta(s,5);return 0xffff;}
    respuesta(s,0,1500);
    if(escritura){if(prueba){activo=false;sesion=0;estado=0;reiniciar=true;}else confirmar=true;}
    return 0xffff;
  }
  // Llamar solo después de transmitir la respuesta RDM completa.
  void ejecutarPendiente(){
    if(estado==1){if(!prueba)memoria.invalidar();estado=2;return;}
    if(estado==3){
      for(uint8_t i=0;i<cantidad;++i){
        const uint32_t offset=desplazamiento+i;const uint8_t b=cola[i];
        crcArchivo=crcByte(crcArchivo,b);
        if(offset<16){if(b!=cabecera[offset]){fallo=9;estado=6;return;}}
        else {
          const uint32_t a=offset-16;pagina[a%256]=b;crcImagen=crc32Byte(crcImagen,b);
          if(a%256==255 || offset==total-1){
            if(!prueba && !memoria.escribir(a&~255UL,pagina)){fallo=0x0d;estado=6;return;}
            memset(pagina,0xff,256);
          }
        }
      }
      ultimoOffset=desplazamiento;longitudUltimo=cantidad;memcpy(ultimo,cola,cantidad);
      recibidos+=cantidad;cantidad=0;estado=recibidos==total?4:2;return;
    }
    if(estado==4){
      if(crcArchivo!=crcEsperado){fallo=12;estado=6;return;}
      // Un byte por iteración: seguir atendiendo la UART durante la comprobación.
      if(!prueba && posicionValidacion<total-16){
        crcFlash=crc32Byte(crcFlash,memoria.leer(posicionValidacion++));return;
      }
      if((crcImagen^0xffffffffUL)!=leer32(cabecera+12) ||
         (!prueba && (crcFlash^0xffffffffUL)!=leer32(cabecera+12))){fallo=0x0d;estado=6;return;}
      validado=true;estado=5;return;
    }
    if(confirmar){memoria.validar();confirmar=false;reiniciar=true;}
  }
};
}
