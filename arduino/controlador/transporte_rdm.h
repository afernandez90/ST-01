#pragma once
#include "rdm.h"
#include "traza_rdm.h"

// Adaptador automático DollaTek: no hay señal DE externa.
// Para un transceptor manual, configurar false y unir DE y /RE al pin 27.
#ifdef ST01_RS485_MANUAL
constexpr bool DIRECCION_RS485_AUTOMATICA = false;
#else
constexpr bool DIRECCION_RS485_AUTOMATICA = true;
#endif
constexpr uint8_t PIN_DIRECCION_RS485 = 27;
RespondedorRdm rdm;
uint8_t entradaRdm[257];
uint16_t indiceRdm = 0;
uint16_t indiceSalidaRdm = 0;
volatile uint16_t respuestasTerminadasVentana=0;
enum class FaseRdm : uint8_t { Reposo, Break, Datos, UltimoByte };
volatile FaseRdm faseRdm = FaseRdm::Reposo;

void iniciarRespuestaRdm() {
  // RX completa antes del final del segundo bit de parada: dejar margen adicional.
  // Espera acotada; la trama se transmite después mediante interrupciones.
  delayMicroseconds(200);
  UCSR1B = 0;
  if (!DIRECCION_RS485_AUTOMATICA) digitalWrite(PIN_DIRECCION_RS485, HIGH);
  UCSR1A = _BV(TXC1);
  indiceSalidaRdm = 0;
  if (rdm.descubrimiento) {
    faseRdm = FaseRdm::Datos;
    UCSR1B = _BV(TXEN1) | _BV(UDRIE1);
  } else {
    // Cero a 50 kbaud, 8N2: break de 180 us y marca de 40 us.
    // La UART mantiene la marca hasta entrar en la interrupción TX.
    faseRdm = FaseRdm::Break;
    UBRR1L = 19;
    UCSR1B = _BV(TXEN1) | _BV(TXCIE1);
    UDR1 = 0;
  }
}

ISR(USART1_UDRE_vect) {
  UDR1 = rdm.salida[indiceSalidaRdm++];
  if (indiceSalidaRdm == rdm.longitudSalida) {
    faseRdm = FaseRdm::UltimoByte;
    // Borrar TXC antes de esperar el último bit, sin desactivar el transmisor.
    UCSR1A = _BV(TXC1);
    UCSR1B = _BV(TXEN1) | _BV(TXCIE1);
  }
}

ISR(USART1_TX_vect) {
  if (faseRdm == FaseRdm::Break) {
    UBRR1L = 3;
    faseRdm = FaseRdm::Datos;
    UCSR1B = _BV(TXEN1) | _BV(UDRIE1);
  } else {
    // Soltar el bus solo cuando han salido todos los bits de parada.
    UCSR1B = 0;
    if (!DIRECCION_RS485_AUTOMATICA) digitalWrite(PIN_DIRECCION_RS485, LOW);
    TrazaRdm::registrar(rdm.descubrimiento?TrazaRdm::TxDescubrimiento:TrazaRdm::Tx,
                        rdm.salida,rdm.longitudSalida);
    ++respuestasTerminadasVentana;
    faseRdm = FaseRdm::Reposo;
    UCSR1B = _BV(RXEN1) | _BV(RXCIE1);
  }
}
