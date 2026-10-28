// Cargador Mega2560 de 8 KiB: FTC por USART1 y carga USB STK500v2 por USART0.
// Nunca se instala automáticamente desde la compilación del sketch.
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "../controlador/rdm.h"
#include "../controlador/ftc.h"

// Con LTO, conservar explícitamente el código de inicio no llamado desde C++.
extern "C" void desactivarWatchdog() __attribute__((used,externally_visible,naked,section(".init3")));
void desactivarWatchdog(){MCUSR=0;wdt_disable();}
extern const uint8_t firma[16] __attribute__((used,section(".firma"))) = {'S','T','0','1','R','D','M','1',1,0,0,0,0,0,0,0};
struct MemoriaFlash {
  void invalidar(){eeprom_update_byte((uint8_t*)Ftc::EEPROM_ESTADO,Ftc::PENDIENTE);}
  void validar(){eeprom_update_byte((uint8_t*)Ftc::EEPROM_ESTADO,Ftc::VALIDA);}
  uint8_t leer(uint32_t p){return pgm_read_byte_far(p);}
  bool escribir(uint32_t p,const uint8_t *datos){
    if(p%256 || p>=Ftc::LIMITE_FLASH)return false;
    eeprom_busy_wait();boot_page_erase(p);boot_spm_busy_wait();
    for(uint16_t i=0;i<256;i+=2)boot_page_fill(p+i,uint16_t(datos[i])|(uint16_t(datos[i+1])<<8));
    boot_page_write(p);boot_spm_busy_wait();boot_rww_enable();
    for(uint16_t i=0;i<256;++i)if(leer(p+i)!=datos[i])return false;
    return true;
  }
} memoria;
Ftc::Receptor<MemoriaFlash> ftc(memoria,true,true);
RespondedorRdm rdm;
uint16_t manejarFtc(uint16_t pid,bool w,const uint8_t *p,uint8_t n,uint8_t *s,uint8_t &l){return ftc.comando(pid,w,p,n,s,l);}
void saltarAplicacion() __attribute__((noreturn));
void saltarAplicacion(){
  cli();UCSR0B=UCSR1B=0;TCCR0A=TCCR0B=0;EIND=0;RAMPZ=0;
  asm volatile("jmp 0");__builtin_unreachable();
}
void ponerUart0(uint8_t b){while(!(UCSR0A&_BV(UDRE0))){}UDR0=b;}
void ponerUart1(uint8_t b){while(!(UCSR1A&_BV(UDRE1))){}UDR1=b;}
void transmitirRdm(){
  _delay_us(200);UCSR1B=_BV(TXEN1);UCSR1A=_BV(TXC1);
#ifdef ST01_RS485_MANUAL
  PORTA|=_BV(PA5); // Pin 27: DE y /RE unidos.
#endif
  if(!rdm.descubrimiento){
    UBRR1L=19;ponerUart1(0);while(!(UCSR1A&_BV(TXC1))){}UCSR1A=_BV(TXC1);UBRR1L=3;
  }
  for(uint16_t i=0;i<rdm.longitudSalida;++i)ponerUart1(rdm.salida[i]);
  while(!(UCSR1A&_BV(TXC1))){}UCSR1B=0;
#ifdef ST01_RS485_MANUAL
  PORTA&=~_BV(PA5);
#endif
  UCSR1B=_BV(RXEN1);
}

#include "usb.h"

int main(){
  cli();
  // Salidas de potencia apagadas; /G de los dígitos deshabilitado.
  DDRE|=_BV(PE3)|_BV(PE4)|_BV(PE5);PORTE&=~(_BV(PE3)|_BV(PE4)|_BV(PE5));
  DDRG|=_BV(PG5);PORTG&=~_BV(PG5);
  DDRH|=_BV(PH3)|_BV(PH4)|_BV(PH5)|_BV(PH6);PORTH&=~(_BV(PH3)|_BV(PH4)|_BV(PH5)|_BV(PH6));
  DDRL|=_BV(PL1);PORTL|=_BV(PL1); // Mega 48.
#ifdef ST01_RS485_MANUAL
  DDRA|=_BV(PA5);PORTA&=~_BV(PA5);
#endif
  UCSR0A=_BV(U2X0);UBRR0H=0;UBRR0L=16;UCSR0C=_BV(UCSZ01)|_BV(UCSZ00);UCSR0B=_BV(RXEN0)|_BV(TXEN0);
  UCSR1A=0;UBRR1H=0;UBRR1L=3;UCSR1C=_BV(USBS1)|_BV(UCSZ11)|_BV(UCSZ10);UCSR1B=_BV(RXEN1);
  TCCR0A=_BV(WGM01);TCCR0B=_BV(CS01)|_BV(CS00);OCR0A=249;
  const uint8_t marca=eeprom_read_byte((uint8_t*)Ftc::EEPROM_ESTADO);
  const bool recuperar=marca==Ftc::PENDIENTE || marca==Ftc::SOLICITUD;
  if(marca==Ftc::SOLICITUD && memoria.leer(0)!=0xff)memoria.validar();
  uint16_t tiempo=0;rdm.gestorFtc=manejarFtc;rdm.etiquetaVersion="ST01 cargador 1.0";
  const uint16_t direccion=eeprom_read_word((uint16_t*)2);
  if(direccion>=1 && direccion<=494)rdm.direccion=direccion;
  uint8_t entrada[257];uint16_t indice=0;bool esperaInicio=false,recibiendo=false;
  for(;;){
    if(TIFR0&_BV(OCF0A)){TIFR0=_BV(OCF0A);if(tiempo<1500)++tiempo;}
    if(UCSR0A&_BV(RXC0))recibirUsb(UDR0);
    if(UCSR1A&_BV(RXC1)){
      const uint8_t estado=UCSR1A,b=UDR1;
      if(estado&_BV(FE1)){esperaInicio=b==0;recibiendo=false;indice=0;}
      else if(estado&(_BV(DOR1)|_BV(UPE1))){esperaInicio=false;recibiendo=false;}
      else if(esperaInicio){esperaInicio=false;recibiendo=b==0xcc;if(recibiendo)entrada[indice++]=b;}
      else if(recibiendo){
        entrada[indice++]=b;
        if(indice==3 && (b<24 || entrada[1]!=1))recibiendo=false;
        if(indice>=3 && indice==uint16_t(entrada[2])+2){recibiendo=false;if(rdm.procesar(entrada,indice))transmitirRdm();}
        if(indice==sizeof(entrada))recibiendo=false;
      }
    }
    ftc.ejecutarPendiente();
    if(ftc.reiniciar && eeprom_read_byte((uint8_t*)Ftc::EEPROM_ESTADO)==Ftc::VALIDA && memoria.leer(0)!=0xff)saltarAplicacion();
    if(!recuperar && !usbActivo && !ftc.activo && tiempo>=1500 && eeprom_read_byte((uint8_t*)Ftc::EEPROM_ESTADO)!=Ftc::PENDIENTE && memoria.leer(0)!=0xff)saltarAplicacion();
  }
}
