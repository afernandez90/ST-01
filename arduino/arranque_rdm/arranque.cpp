// Cargador Mega2560 de 8 KiB: FTC por USART1 y carga USB STK500v2 por USART0.
// Nunca se instala automáticamente desde la compilación del sketch.
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "../pruebas/recepcion_dmx/rdm.h"
#include "../pruebas/recepcion_dmx/ftc.h"

void desactivarWatchdog() __attribute__((naked,section(".init3")));
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

uint8_t usb[285];uint16_t usbIndice=0,usbTamano=0;uint8_t usbSecuencia=0,usbSuma=0,usbFase=0;
uint32_t direccionUsb=0;bool usbActivo=false,usbSalir=false,usbEscrito=false,usbError=false;
void respuestaUsb(uint16_t n){
  UCSR0A=_BV(U2X0)|_BV(TXC0);
  uint8_t suma=0x1b^usbSecuencia^uint8_t(n>>8)^uint8_t(n)^0x0e;
  ponerUart0(0x1b);ponerUart0(usbSecuencia);ponerUart0(n>>8);ponerUart0(n);ponerUart0(0x0e);
  for(uint16_t i=0;i<n;++i){suma^=usb[i];ponerUart0(usb[i]);}ponerUart0(suma);
}
void comandoUsb(){
  const uint8_t cmd=usb[0];uint16_t respuesta=2;bool ok=true;
  if(!usbTamano)return;
  if(cmd==1){usb[1]=0;usb[2]=8;memcpy(usb+3,"AVRISP_2",8);respuesta=11;}
  else if(cmd==3 && usbTamano>=2){uint8_t p=usb[1];usb[1]=0;usb[2]=p==0x90?15:p==0x91?2:p==0x92?10:0;respuesta=3;}
  else if(cmd==6 && usbTamano==5){direccionUsb=(Ftc::leer32(usb+1)&0x7fffffffUL)*2;}
  else if(cmd==0x10 || cmd==2){}
  else if(cmd==0x11){usbSalir=true;}
  else if(cmd==0x12){ok=false;} // Borrado global no disponible por auto-programación.
  else if(cmd==0x1b && usbTamano>=5){const uint8_t sig[]={0x1e,0x98,1};usb[2]=sig[usb[4]<3?usb[4]:2];usb[3]=0;respuesta=4;}
  else if((cmd==0x18 || cmd==0x1a) && usbTamano>=4){
    usb[2]=cmd==0x1a?boot_lock_fuse_bits_get(GET_LOCK_BITS):
        usb[2]==0x50?(usb[3]==8?boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS):boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS)):
        boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    usb[3]=0;respuesta=4;
  }
  else if(cmd==0x1d && usbTamano>=7){
    const uint8_t a=usb[4],b=usb[5],c=usb[6];uint8_t v=0;
    if(a==0x30){const uint8_t sig[]={0x1e,0x98,1};v=c<3?sig[c]:0;}
    else if(a==0x50)v=b==8?boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS):boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    else if(a==0x58)v=b==8?boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS):boot_lock_fuse_bits_get(GET_LOCK_BITS);
    usb[2]=0;usb[3]=a;usb[4]=b;usb[5]=v;usb[6]=0;respuesta=7;
  }
  else if(cmd==0x13 && usbTamano>=10){
    const uint16_t n=Ftc::leer16(usb+1);
    if(!n || n>256 || usbTamano!=n+10 || direccionUsb%256 || direccionUsb+n>Ftc::LIMITE_FLASH)ok=false;
    else{
      uint8_t pagina[256];memset(pagina,0xff,256);memcpy(pagina,usb+10,n);
      memoria.invalidar();usbEscrito=true;ok=memoria.escribir(direccionUsb,pagina);direccionUsb+=n;
    }
  }
  else if(cmd==0x14 && usbTamano>=10){
    const uint16_t n=Ftc::leer16(usb+1);const uint32_t base=direccionUsb/2;
    if(usbTamano!=n+10 || base+n>4096)ok=false;
    else{for(uint16_t i=0;i<n;++i)eeprom_update_byte((uint8_t*)(uint16_t)(base+i),usb[10+i]);direccionUsb+=uint32_t(n)*2;}
  }
  else if((cmd==0x15 || cmd==0x16) && usbTamano>=3){
    const uint16_t n=Ftc::leer16(usb+1);const uint32_t base=cmd==0x15?direccionUsb:direccionUsb/2;
    if(n>256 || base+n>(cmd==0x15?Ftc::LIMITE_FLASH:4096))ok=false;
    else{for(uint16_t i=0;i<n;++i)usb[2+i]=cmd==0x15?memoria.leer(base+i):eeprom_read_byte((uint8_t*)(uint16_t)(base+i));usb[n+2]=0;respuesta=n+3;direccionUsb+=cmd==0x15?n:uint32_t(n)*2;}
  }else ok=false;
  if(!ok && (cmd==0x13 || cmd==0x14))usbError=true;
  usb[1]=ok?0:0xc0;respuestaUsb(respuesta);
  if(usbSalir){
    while(!(UCSR0A&_BV(TXC0))){}
    if(usbEscrito && !usbError && ok)memoria.validar();
    if(eeprom_read_byte((uint8_t*)Ftc::EEPROM_ESTADO)!=Ftc::PENDIENTE && memoria.leer(0)!=0xff)saltarAplicacion();
    usbSalir=false;
  }
}
void recibirUsb(uint8_t b){
  switch(usbFase){
    case 0:if(b==0x1b){usbSuma=b;usbFase=1;}return;
    case 1:usbSecuencia=b;usbFase=2;break;
    case 2:usbTamano=uint16_t(b)<<8;usbFase=3;break;
    case 3:usbTamano|=b;usbFase=usbTamano && usbTamano<=sizeof(usb)?4:0;break;
    case 4:usbFase=b==0x0e?5:0;usbIndice=0;break;
    case 5:usb[usbIndice++]=b;if(usbIndice==usbTamano)usbFase=6;break;
    case 6:usbFase=0;if((usbSuma^b)==0){usbActivo=true;comandoUsb();}return;
  }usbSuma^=b;
}
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
