#pragma once
#include "ftc.h"
#ifndef ST01_SIMULACION
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#endif
struct MemoriaAplicacion {
  void invalidar(){}
  void validar(){}
  uint8_t leer(uint32_t){return 0xff;}
  bool escribir(uint32_t,const uint8_t*){return false;}
} memoriaAplicacion;
Ftc::Receptor<MemoriaAplicacion> actualizacion(memoriaAplicacion,false,false);
bool detectarArranqueRdm(){
#ifdef ST01_SIMULACION
  return false;
#else
  const char firma[]="ST01RDM1";
  for(uint8_t i=0;i<8;++i)if(pgm_read_byte_far(Ftc::DIRECCION_FIRMA+i)!=uint8_t(firma[i]))return false;
  return true;
#endif
}
uint16_t gestionarFtc(uint16_t pid,bool escribir,const uint8_t *p,uint8_t n,uint8_t *s,uint8_t &l){
  return actualizacion.comando(pid,escribir,p,n,s,l);
}
void comprobarReinicioRdm(){
  if(!actualizacion.reiniciar || faseRdm!=FaseRdm::Reposo)return;
  // La respuesta ya terminó: dejar de aceptar tráfico y vaciar su traza antes del reset.
  UCSR1B=0;
  if(TrazaRdm::salida!=TrazaRdm::entrada)return;
  // La confirmación RDM ya ha salido del puerto antes de apagar y reiniciar.
  EstadoSalidas apagado={};aplicarSalidasHardware(apagado);
  FastLED.clear(true);
  digitalWrite(PIN_DIGITOS_HABILITAR,HIGH);
  EEPROM.update(Ftc::EEPROM_ESTADO,Ftc::SOLICITUD);
  Serial.println(F("[RDM] Reinicio al cargador FTC. Salidas apagadas."));
  Serial.flush();
#ifndef ST01_SIMULACION
  wdt_enable(WDTO_15MS);for(;;){}
#endif
}
