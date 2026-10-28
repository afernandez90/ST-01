#pragma once
#include "salidas.h"
#include "digitos.h"
#include "efectos.h"
// Conservar la recepción UART; las tiras requieren una prueba física de temporización.
#define FASTLED_ALLOW_INTERRUPTS 1
#include <FastLED.h>

constexpr uint8_t PINES_PILOTOS[] = {2,3,4,5,6,7,8};
constexpr uint8_t PIN_ESTADO = 9;
constexpr uint8_t PIN_BARRA = 10, PIN_TIRA = 11;
constexpr uint8_t PIN_DIGITOS_DATOS = 51, PIN_DIGITOS_RELOJ = 52;
constexpr uint8_t PIN_DIGITOS_LATCH = 49, PIN_DIGITOS_HABILITAR = 48;
// Barra: 38 píxeles comprobados. Tira: cantidad provisional; contar grupos direccionables.
constexpr uint16_t PIXELES_BARRA = 38, PIXELES_TIRA = 300;
constexpr uint8_t BRILLO_RGB_MAXIMO = 64;
static_assert(PIXELES_BARRA == Efectos::PASOS_BARRA, "Actualizar mapa DMX de progreso al cambiar la barra");
CRGB pixelesBarra[PIXELES_BARRA], pixelesTira[PIXELES_TIRA];

void iniciarSalidasHardware() {
  for (uint8_t pin : PINES_PILOTOS) { digitalWrite(pin,LOW); pinMode(pin,OUTPUT); }
  digitalWrite(PIN_ESTADO,LOW); pinMode(PIN_ESTADO,OUTPUT);
  // /G en HIGH mantiene apagados los TPIC6B595 mientras se inicializan.
  digitalWrite(PIN_DIGITOS_HABILITAR,HIGH); pinMode(PIN_DIGITOS_HABILITAR,OUTPUT);
  const uint8_t pinesDigitos[] = {PIN_DIGITOS_DATOS,PIN_DIGITOS_RELOJ,PIN_DIGITOS_LATCH};
  for (uint8_t pin : pinesDigitos) {
    digitalWrite(pin,LOW); pinMode(pin,OUTPUT);
  }
  for (uint8_t i=0;i<4;++i) shiftOut(PIN_DIGITOS_DATOS,PIN_DIGITOS_RELOJ,MSBFIRST,0);
  digitalWrite(PIN_DIGITOS_LATCH,HIGH);digitalWrite(PIN_DIGITOS_LATCH,LOW);
  digitalWrite(PIN_DIGITOS_HABILITAR,LOW);
  FastLED.addLeds<WS2812B,PIN_BARRA,GRB>(pixelesBarra,PIXELES_BARRA);
  FastLED.addLeds<WS2811,PIN_TIRA,RGB>(pixelesTira,PIXELES_TIRA);
  FastLED.setBrightness(BRILLO_RGB_MAXIMO);
}
void aplicarSalidasHardware(const EstadoSalidas &estado) {
  for (uint8_t i=0;i<7;++i) analogWrite(PINES_PILOTOS[i],estado.canales[8+i]);
  digitalWrite(PIN_ESTADO,estado.pilotoEstado ? HIGH : LOW);
  analogWrite(LED_BUILTIN,estado.canales[12]);
  static uint8_t anteriores[4] = {255,255,255,255};
  bool distintos=false;
  for(uint8_t i=0;i<4;++i) if(anteriores[i]!=estado.canales[i]) distintos=true;
  if (distintos) {
    digitalWrite(PIN_DIGITOS_LATCH,LOW);
    // Millares cerca del Mega; unidades al final de la cadena.
    for(int8_t i=3;i>=0;--i) shiftOut(PIN_DIGITOS_DATOS,PIN_DIGITOS_RELOJ,MSBFIRST,
                                     mascaraDigito(estado.canales[i]));
    digitalWrite(PIN_DIGITOS_LATCH,HIGH);digitalWrite(PIN_DIGITOS_LATCH,LOW);
    memcpy(anteriores,estado.canales,4);
  }
  static unsigned long ultimoEnvio=0;
  // Refresco de 20 Hz; no transmitir durante las respuestas RDM.
  if (millis()-ultimoEnvio>=50 && faseRdm==FaseRdm::Reposo) {
    ultimoEnvio=millis();
    static Efectos::Reloj relojBarra, relojTira;
    const uint32_t ahora=millis();
    const uint8_t modos[]={estado.canales[7],estado.canales[18]};
    const uint32_t tiempos[]={relojBarra.tiempo(modos[0],ahora),relojTira.tiempo(modos[1],ahora)};
    for(uint8_t tira=0;tira<2;++tira) {
      CRGB *pixeles=tira ? pixelesTira : pixelesBarra;
      const uint16_t cantidad=tira ? PIXELES_TIRA : PIXELES_BARRA;
      const uint8_t rgb=tira ? 15 : 4;
      const Efectos::Modo modo=Efectos::decodificar(modos[tira],!tira);
      for(uint16_t i=0;i<cantidad;++i) {
        const uint8_t brillo=Efectos::intensidad(modo,i,cantidad,tiempos[tira]);
        pixeles[i]=CRGB(uint16_t(estado.canales[rgb])*brillo/255,
                        uint16_t(estado.canales[rgb+1])*brillo/255,
                        uint16_t(estado.canales[rgb+2])*brillo/255);
      }
    }
    FastLED.show();
  }
}
