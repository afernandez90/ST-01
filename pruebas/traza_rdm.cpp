// Captura diferida, validación y saturación del registro USB de RDM.
#include <cassert>
#include <vector>
#include <iostream>
#include "../arduino/pruebas/recepcion_dmx/traza_rdm.h"
std::vector<uint8_t> trama(){
  const uint8_t propio[6]=ST01_UID_BYTES;
  std::vector<uint8_t> p(26);p[0]=0xcc;p[1]=1;p[2]=24;
  memcpy(p.data()+3,propio,6);
  const uint8_t controlador[]={0x45,0x4e,2,0x35,8,0x11};memcpy(p.data()+9,controlador,6);
  p[15]=37;p[16]=1;p[20]=0x20;p[21]=0;p[22]=0xf0;
  uint16_t suma=0;for(unsigned i=0;i<p.size()-2;++i)suma+=p[i];
  p[24]=suma>>8;p[25]=suma;return p;
}
void vaciar(){while(TrazaRdm::salida!=TrazaRdm::entrada)TrazaRdm::procesar();}
int main(){
  auto p=trama();reloj=1234;
  TrazaRdm::registrar(TrazaRdm::Rx,p.data(),p.size(),1);
  assert(Serial.texto.empty()); // Ningún acceso USB en la captura de interrupción.
  p[9]=0; // La instantánea no depende del búfer del receptor.
  TrazaRdm::procesar();
  assert(Serial.texto.find("[RDM RX #1 t=1234ms]")!=std::string::npos);
  assert(Serial.texto.find("origen=454E:02350811")!=std::string::npos);
  assert(Serial.texto.find("destino=7FF0:66601337")!=std::string::npos);
  assert(Serial.texto.find("PID=0x00F0(DIRECCION_DMX)")!=std::string::npos);
  assert(Serial.texto.find("RESPUESTA_PREPARADA")!=std::string::npos);
  p=trama();p.back()^=1;
  TrazaRdm::registrar(TrazaRdm::Rx,p.data(),p.size());vaciar();
  assert(Serial.texto.find("SUMA_INVALIDA")!=std::string::npos);
  p=trama();p[23]=5;
  TrazaRdm::registrar(TrazaRdm::Rx,p.data(),p.size());vaciar();
  assert(Serial.texto.find("FORMATO_INVALIDO")!=std::string::npos);
  p=trama();p.resize(28);p[2]=26;p[20]=0x21;p[16]=2;p[23]=2;p[24]=0;p[25]=6;
  uint16_t suma=0;for(unsigned i=0;i<p.size()-2;++i)suma+=p[i];p[26]=suma>>8;p[27]=suma;
  TrazaRdm::registrar(TrazaRdm::Tx,p.data(),p.size());vaciar();
  assert(Serial.texto.find("respuesta=NACK motivo=0x0006")!=std::string::npos);
  for(auto tipo:{TrazaRdm::CabeceraInvalida,TrazaRdm::CorteBreak,TrazaRdm::TiempoAgotado,TrazaRdm::ErrorUart}){
    TrazaRdm::registrar(tipo,p.data(),3,2);vaciar();
  }
  assert(Serial.texto.find("CABECERA_INVALIDA")!=std::string::npos);
  assert(Serial.texto.find("INCOMPLETA_NUEVO_BREAK")!=std::string::npos);
  assert(Serial.texto.find("INCOMPLETA_TIEMPO_AGOTADO")!=std::string::npos);
  assert(Serial.texto.find("ERROR_UART_0x02")!=std::string::npos);
  const uint8_t propio[6]=ST01_UID_BYTES;uint8_t d[24]={};memset(d,0xfe,7);d[7]=0xaa;
  for(unsigned i=0;i<6;++i){d[8+i*2]=propio[i]|0xaa;d[9+i*2]=propio[i]|0x55;}
  TrazaRdm::registrar(TrazaRdm::TxDescubrimiento,d,24);vaciar();
  assert(Serial.texto.find("DESCUBRIMIENTO_CODIFICADO | UID=7FF0:66601337")!=std::string::npos);
  p=trama();Serial.texto.clear();
  for(unsigned i=0;i<TrazaRdm::CAPACIDAD+3;++i)TrazaRdm::registrar(TrazaRdm::Rx,p.data(),p.size(),1);
  assert(TrazaRdm::perdidos==4);vaciar();
  assert(Serial.texto.find("eventos omitidos: 4")!=std::string::npos);
  assert(TrazaRdm::perdidos==0);
  std::cout<<"Traza RDM: captura sin USB, UID, bytes, NACK, errores y saturacion correctos.\n";
}
