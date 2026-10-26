// Simulación de UART, EEPROM, LCD y salida; no mide tiempos eléctricos reales.
#define ST01_SIMULACION
#include <cassert>
#include <vector>
#include <iostream>
#include "../arduino/controlador/controlador.ino"
void recibir(uint8_t b, uint8_t estado=0) { UCSR1A=estado; UDR1=b; USART1_RX_vect(); }
void dmx(uint16_t objetivo, uint8_t valor) {
  recibir(0, _BV(FE1)); recibir(0);
  for (int i=1; i<=512; ++i) recibir(i==objetivo ? valor : 0);
}
void rdmRecibir(uint16_t pid, std::vector<uint8_t> datos={}, bool escritura=false) {
  std::vector<uint8_t> p(26+datos.size());
  p[0]=0xcc; p[1]=1; p[2]=p.size()-2;
  memcpy(p.data()+3, rdm.uid, 6);
  p[9]=0x12; p[10]=0x34; p[14]=2; p[15]=1; p[16]=1;
  p[20]=escritura ? 0x30 : 0x20;
  RespondedorRdm::escribir16(p.data()+21, pid);
  p[23]=datos.size(); memcpy(p.data()+24,datos.data(),datos.size());
  uint16_t suma=0; for(unsigned i=0;i<p.size()-2;++i) suma+=p[i];
  RespondedorRdm::escribir16(p.data()+p.size()-2,suma);
  recibir(0,_BV(FE1)); for(auto b:p) recibir(b);
}
void terminarRespuesta(bool descubrimiento=false) {
  if (!descubrimiento) {
    assert(faseRdm==FaseRdm::Break && UBRR1L==19);
    assert(UDR1==0 && !(UCSR1B & _BV(RXEN1)));
    USART1_TX_vect();
  }
  assert(faseRdm==FaseRdm::Datos && UBRR1L==3);
  for(unsigned i=0;i<rdm.longitudSalida;++i) {
    USART1_UDRE_vect(); assert(UDR1==rdm.salida[i]);
  }
  assert(faseRdm==FaseRdm::UltimoByte && !(UCSR1B & _BV(RXEN1)));
  USART1_TX_vect();
  assert(faseRdm==FaseRdm::Reposo && (UCSR1B & _BV(RXEN1)));
  assert(respuestasTerminadasVentana>0);
}
int main() {
  setup();
  dmx(13,123); reloj=10; loop(); assert(salida==123);
  rdmRecibir(0x60); assert(esperaMicros==200); terminarRespuesta();
  assert(salida==123);  // Consultar RDM no interrumpe la salida DMX.
  rdmRecibir(0xf0,{1,238},true); // Dirección 494, último inicio permitido.
  assert(direccionDmx==494 && rdm.direccion==494); terminarRespuesta();
  reloj=20;loop();assert(direccionPendiente==494 && salida==0);
  Configuracion config; EEPROM.get(0,config);assert(config.direccion==494);
  dmx(506,177); reloj=300;loop(); assert(salida==177);
  assert(pantalla.lineas[0]=="ST-01 DMX: 494  ");
  assert(pantalla.lineas[1]=="DMX OK  LNB:177 ");
  rdmRecibir(0x1000,{1},true);terminarRespuesta();reloj=550;loop();assert(salida==0);
  for(auto pin:PINES_PILOTOS)assert(pinesAnalogicos[pin]==0);
  assert(!pinesDigitales[PIN_ESTADO] && !pantalla.iluminada);
  for(unsigned i=0;i<4;++i)assert(desplazados[(cuantosDesplazados-1-i)%1024]==0);
  assert(pantalla.lineas[0]=="IDENTIFICAR ST01");
  reloj=800;loop();assert(salida==255);
  for(auto pin:PINES_PILOTOS)assert(pinesAnalogicos[pin]==255);
  assert(pinesDigitales[PIN_ESTADO] && pantalla.iluminada);
  for(unsigned i=0;i<4;++i)assert(desplazados[(cuantosDesplazados-1-i)%1024]==0xff);
  for(auto pixel:pixelesBarra)assert(pixel.r==255 && pixel.g==255 && pixel.b==255);
  for(auto pixel:pixelesTira)assert(pixel.r==255 && pixel.g==255 && pixel.b==255);
  rdmRecibir(0x1000,{0},true);terminarRespuesta();reloj=850;loop();assert(salida==177);
  reloj=1400;loop();assert(salida==0);
  assert(pantalla.lineas[1]=="SIN DMX LNB:177 ");
  rdmRecibir(0x82,{'P','r','u','e','b','a'},true);terminarRespuesta();loop();
  assert(EEPROM.read(32)==0x52 && EEPROM.read(33)=='P');
  // La respuesta de descubrimiento omite el break, pero espera el último bit.
  rdm.descubrimiento=true;rdm.longitudSalida=24;iniciarRespuestaRdm();
  terminarRespuesta(true);
  while(TrazaRdm::salida!=TrazaRdm::entrada)TrazaRdm::procesar();
  recibir(0,_BV(FE1));recibir(0xcc);recibir(1);recibir(24);
  reloj+=101;loop();while(TrazaRdm::salida!=TrazaRdm::entrada)TrazaRdm::procesar();
  assert(Serial.texto.find("INCOMPLETA_TIEMPO_AGOTADO")!=std::string::npos);
  recibir(0,_BV(FE1));recibir(0xcc);recibir(1);recibir(5);
  TrazaRdm::procesar();assert(Serial.texto.find("CABECERA_INVALIDA")!=std::string::npos);
  // Conservar las trazas de la última respuesta antes de solicitar el cargador.
  uint8_t parcial[]={0xcc,1,24};
  TrazaRdm::registrar(TrazaRdm::CabeceraInvalida,parcial,3);
  actualizacion.reiniciar=true;faseRdm=FaseRdm::Reposo;
  comprobarReinicioRdm();assert(EEPROM.read(Ftc::EEPROM_ESTADO)!=Ftc::SOLICITUD);
  while(TrazaRdm::salida!=TrazaRdm::entrada)TrazaRdm::procesar();
  comprobarReinicioRdm();assert(EEPROM.read(Ftc::EEPROM_ESTADO)==Ftc::SOLICITUD);
  // Fin del reinicio simulado; volver al funcionamiento normal.
  actualizacion.reiniciar=false;
  // Integración de efectos: la trama DMX entrega RGB y ambos selectores.
  uint8_t trama[CANALES_ST01]={};
  trama[4]=200;trama[5]=100;trama[6]=50;trama[7]=30;
  trama[15]=40;trama[16]=80;trama[17]=120;trama[18]=0;
  recibir(0,_BV(FE1));recibir(0);
  for(uint16_t canal=1;canal<=512;++canal)
    recibir(canal>=494 ? trama[canal-494] : 0);
  reloj=1600;loop();
  for(unsigned i=0;i<38;++i) {
    assert(pixelesBarra[i].r==(i<30 ? 200 : 0));
    assert(pixelesBarra[i].g==(i<30 ? 100 : 0));
    assert(pixelesBarra[i].b==(i<30 ? 50 : 0));
  }
  for(auto pixel:pixelesTira)assert(pixel.r==40 && pixel.g==80 && pixel.b==120);
  trama[7]=111; // KITT lento, comienza en el primer píxel.
  reloj=1650;aplicarSalidasHardware(calcularSalidas(trama,true,false,reloj));
  assert(pixelesBarra[0].r==200 && pixelesBarra[1].r==120);
  assert(pixelesBarra[2].r==0 && pixelesBarra[37].r==0);
  trama[4]=100; // Cambiar RGB conserva el reloj del barrido.
  reloj=3150;aplicarSalidasHardware(calcularSalidas(trama,true,false,reloj));
  assert(pixelesBarra[0].r==0);
  reloj=3200;aplicarSalidasHardware(calcularSalidas(trama,false,false,reloj));
  for(auto pixel:pixelesBarra)assert(pixel.r==0 && pixel.g==0 && pixel.b==0);
  for(auto pixel:pixelesTira)assert(pixel.r==0 && pixel.g==0 && pixel.b==0);
  reloj=3250;aplicarSalidasHardware(calcularSalidas(trama,false,true,reloj));
  for(auto pixel:pixelesBarra)assert(pixel.r==255 && pixel.g==255 && pixel.b==255);
  for(auto pixel:pixelesTira)assert(pixel.r==255 && pixel.g==255 && pixel.b==255);
  std::cout<<"UART: break, datos, retorno RX, direccion, EEPROM, LCD, identificar y perdida DMX correctos.\n";
}
