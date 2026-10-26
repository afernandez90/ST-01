#pragma once
#include <Arduino.h>
#include <string.h>
#include "identidad.h"
#include <util/atomic.h>

// Cola acotada: las interrupciones capturan bytes; solo loop() imprime por USB.
namespace TrazaRdm {
enum Tipo : uint8_t { Rx, Tx, TxDescubrimiento, CabeceraInvalida, CorteBreak, ErrorUart, TiempoAgotado };
struct Evento {
  uint32_t numero, instante;
  uint16_t longitud;
  Tipo tipo;
  uint8_t detalle;
  uint8_t bytes[257];
};
constexpr uint8_t CAPACIDAD=8;
Evento cola[CAPACIDAD];
volatile uint8_t entrada=0,salida=0;
volatile uint16_t perdidos=0;
uint32_t siguienteNumero=0;

// Productor: interrupciones, o sección atómica para declarar una recepción agotada.
void registrar(Tipo tipo,const uint8_t *p,uint16_t n,uint8_t detalle=0){
  const uint32_t numero=++siguienteNumero;
  const uint8_t siguiente=(entrada+1)%CAPACIDAD;
  if(siguiente==salida){if(perdidos!=65535)++perdidos;return;}
  Evento &e=cola[entrada];
  e.numero=numero;e.instante=millis();e.tipo=tipo;e.detalle=detalle;
  e.longitud=n>257?257:n;
  if(e.longitud)memcpy(e.bytes,p,e.longitud);
  // Publicar después de copiar. La barrera evita adelantar la publicación.
  asm volatile("" ::: "memory");
  entrada=siguiente;
}
void hex8(uint8_t b){
  const char digitos[]="0123456789ABCDEF";
  Serial.print(digitos[b>>4]);Serial.print(digitos[b&15]);
}
void hex16(uint16_t n){hex8(n>>8);hex8(n);}
void imprimirUid(const uint8_t *p){
  for(uint8_t i=0;i<6;++i){if(i==2)Serial.print(':');hex8(p[i]);}
}
uint16_t leer16(const uint8_t *p){return (uint16_t(p[0])<<8)|p[1];}
void nombrePid(uint16_t pid){
  switch(pid){
    case 1:Serial.print(F("DESCUBRIR"));break;
    case 2:Serial.print(F("SILENCIAR"));break;
    case 3:Serial.print(F("DESILENCIAR"));break;
    case 0x50:Serial.print(F("PARAMETROS"));break;
    case 0x60:Serial.print(F("INFORMACION_EQUIPO"));break;
    case 0x80:Serial.print(F("MODELO"));break;
    case 0x81:Serial.print(F("FABRICANTE"));break;
    case 0x82:Serial.print(F("ETIQUETA"));break;
    case 0xc0:Serial.print(F("FIRMWARE"));break;
    case 0xe0:Serial.print(F("PERSONALIDAD"));break;
    case 0xe1:Serial.print(F("DESCRIPCION_PERSONALIDAD"));break;
    case 0xf0:Serial.print(F("DIRECCION_DMX"));break;
    case 0x1000:Serial.print(F("IDENTIFICAR"));break;
    case 0x1200:Serial.print(F("FTC_INICIAR"));break;
    case 0x1201:Serial.print(F("FTC_CARGAR"));break;
    case 0x1202:Serial.print(F("FTC_CONFIRMAR"));break;
    case 0x1203:Serial.print(F("FTC_CANCELAR"));break;
    default:Serial.print(F("DESCONOCIDO"));break;
  }
}
void imprimir(const Evento &e){
  Serial.print(F("[RDM "));Serial.print(e.tipo==Tx || e.tipo==TxDescubrimiento?F("TX"):F("RX"));
  Serial.print(F(" #"));Serial.print(e.numero);Serial.print(F(" t="));Serial.print(e.instante);Serial.print(F("ms] "));
  const uint8_t *p=e.bytes;const uint16_t n=e.longitud;
  if(e.tipo==TxDescubrimiento){
    Serial.print(F("DESCUBRIMIENTO_CODIFICADO | UID="));
    if(n==24){uint8_t id[6];for(uint8_t i=0;i<6;++i)id[i]=p[8+2*i]&p[9+2*i];imprimirUid(id);}
    Serial.print(F(" | resultado=TRANSMITIDA"));
  }else{
    bool formato=n>=26 && p[0]==0xcc && p[1]==1 && p[2]>=24 && n==uint16_t(p[2])+2 && p[23]==p[2]-24;
    uint16_t suma=0;
    if(formato)for(uint16_t i=0;i<n-2;++i)suma+=p[i];
    if(n>=24){
      switch(p[20]){
        case 0x10:Serial.print(F("DESCUBRIMIENTO"));break;
        case 0x11:Serial.print(F("RESP_DESCUBRIMIENTO"));break;
        case 0x20:Serial.print(F("GET"));break;
        case 0x21:Serial.print(F("RESP_GET"));break;
        case 0x30:Serial.print(F("SET"));break;
        case 0x31:Serial.print(F("RESP_SET"));break;
        default:Serial.print(F("CLASE=0x"));hex8(p[20]);break;
      }
      Serial.print(F(" | trans="));Serial.print(p[15]);
      Serial.print(F(" | origen="));imprimirUid(p+9);
      Serial.print(F(" | destino="));imprimirUid(p+3);
      Serial.print(F(" | sub="));Serial.print(leer16(p+18));
      Serial.print(F(" | PID=0x"));hex16(leer16(p+21));Serial.print('(');nombrePid(leer16(p+21));Serial.print(')');
      Serial.print(F(" | PDL="));Serial.print(p[23]);
      if(e.tipo==Tx){
        Serial.print(F(" | respuesta="));
        if(p[16]==0)Serial.print(F("ACK"));
        else if(p[16]==2){Serial.print(F("NACK"));if(p[23]>=2 && n>=28){Serial.print(F(" motivo=0x"));hex16(leer16(p+24));}}
        else{Serial.print(F("TIPO_0x"));hex8(p[16]);}
      }
    }
    Serial.print(F(" | resultado="));
    if(e.tipo==CabeceraInvalida)Serial.print(F("CABECERA_INVALIDA"));
    else if(e.tipo==CorteBreak)Serial.print(F("INCOMPLETA_NUEVO_BREAK"));
    else if(e.tipo==ErrorUart){Serial.print(F("ERROR_UART_0x"));hex8(e.detalle);}
    else if(e.tipo==TiempoAgotado)Serial.print(F("INCOMPLETA_TIEMPO_AGOTADO"));
    else if(!formato)Serial.print(F("FORMATO_INVALIDO"));
    else if(suma!=leer16(p+n-2))Serial.print(F("SUMA_INVALIDA"));
    else if(e.tipo==Tx)Serial.print(F("TRANSMITIDA"));
    else if(e.detalle&1)Serial.print(F("RESPUESTA_PREPARADA"));
    else{
      const uint8_t propio[6]=ST01_UID_BYTES;
      const bool fabricante=(p[3]==propio[0] && p[4]==propio[1]) || (p[3]==255 && p[4]==255);
      const bool difusion=fabricante && p[5]==255 && p[6]==255 && p[7]==255 && p[8]==255;
      if(!difusion && memcmp(p+3,propio,6))Serial.print(F("OTRO_UID"));
      else if(p[20]!=0x10 && p[20]!=0x20 && p[20]!=0x30)Serial.print(F("CLASE_NO_ADMITIDA"));
      else if(p[20]==0x10 && leer16(p+21)==1)Serial.print(e.detalle&2?F("DESCUBRIMIENTO_SILENCIADO"):F("FUERA_DEL_INTERVALO"));
      else if(difusion)Serial.print(F("DIFUSION_SIN_RESPUESTA"));
      else Serial.print(F("SIN_RESPUESTA"));
    }
  }
  Serial.print(F(" | bytes="));Serial.print(n);Serial.print(F(" | trama="));
  for(uint16_t i=0;i<n;++i)hex8(p[i]);
  Serial.println();
}
void procesar(){
  uint16_t omitidos;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){omitidos=perdidos;perdidos=0;}
  if(omitidos){Serial.print(F("[RDM TRAZA] Cola USB llena; eventos omitidos: "));Serial.println(omitidos);}
  if(salida==entrada)return;
  // El productor no modifica esta posición hasta liberar el índice de salida.
  Evento e=cola[salida];
  asm volatile("" ::: "memory");
  salida=(salida+1)%CAPACIDAD;
  imprimir(e);
}
}
