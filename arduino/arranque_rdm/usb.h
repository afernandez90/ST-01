#pragma once
// Protocolo STK500v2 (AVR068): flash 0x13/0x14, EEPROM 0x15/0x16.
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
  else if(cmd==0x15 && usbTamano>=10){
    const uint16_t n=Ftc::leer16(usb+1);const uint32_t base=direccionUsb/2;
    if(usbTamano!=n+10 || base+n>4096)ok=false;
    else{for(uint16_t i=0;i<n;++i)eeprom_update_byte((uint8_t*)(uintptr_t)(base+i),usb[10+i]);direccionUsb+=uint32_t(n)*2;}
  }
  else if((cmd==0x14 || cmd==0x16) && usbTamano>=3){
    const uint16_t n=Ftc::leer16(usb+1);const uint32_t base=cmd==0x14?direccionUsb:direccionUsb/2;
    if(n>256 || base+n>(cmd==0x14?Ftc::LIMITE_FLASH:4096))ok=false;
    else{for(uint16_t i=0;i<n;++i)usb[2+i]=cmd==0x14?memoria.leer(base+i):eeprom_read_byte((uint8_t*)(uintptr_t)(base+i));usb[n+2]=0;respuesta=n+3;direccionUsb+=cmd==0x14?n:uint32_t(n)*2;}
  }else ok=false;
  if(!ok && (cmd==0x13 || cmd==0x15))usbError=true;
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
