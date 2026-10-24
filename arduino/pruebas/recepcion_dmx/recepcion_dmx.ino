#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// Entrada DMX por RX1 (pin 19). No utilizar Serial1 con esta interrupción.
constexpr uint16_t DIRECCION_DMX = 1;
constexpr unsigned long TIEMPO_SIN_DMX_MS = 1000;
volatile uint8_t valorRecibido = 0;
volatile bool datoPendiente = false;
volatile uint16_t paquetesRecibidos = 0;

ISR(USART1_RX_vect) {
  const uint8_t estado = UCSR1A;
  const uint8_t dato = UDR1;
  static bool esperaInicio = false;
  static bool paqueteValido = false;
  static uint16_t canal = 0;

  // El break produce un error de trama y un byte cero.
  if (estado & _BV(FE1)) {
    esperaInicio = (dato == 0);
    paqueteValido = false;
    canal = 0;
    return;
  }
  if (estado & (_BV(DOR1) | _BV(UPE1))) {
    esperaInicio = false;
    paqueteValido = false;
    return;
  }
  if (esperaInicio) {
    esperaInicio = false;
    paqueteValido = (dato == 0);  // Ignorar RDM y otros códigos de inicio.
    return;
  }
  if (!paqueteValido) return;
  ++canal;
  if (canal == DIRECCION_DMX) {
    valorRecibido = dato;
    datoPendiente = true;
    ++paquetesRecibidos;
  }
  if (canal >= 512) paqueteValido = false;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  // 16 MHz / (16 * (3 + 1)) = 250000 baudios; formato 8N2.
  UCSR1B = 0;
  UCSR1A = 0;
  UBRR1H = 0;
  UBRR1L = 3;
  UCSR1C = _BV(USBS1) | _BV(UCSZ11) | _BV(UCSZ10);
  UCSR1B = _BV(RXEN1) | _BV(RXCIE1);
  Serial.println(F("Prueba DMX: canal 1, RX1 pin 19, umbral 128."));
}

void loop() {
  static unsigned long ultimaRecepcion = 0;
  static unsigned long ultimoInforme = 0;
  static bool recibido = false;
  static uint8_t valor = 0;
  uint16_t paquetes;
  const unsigned long ahora = millis();
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (datoPendiente) {
      valor = valorRecibido;
      datoPendiente = false;
      recibido = true;
      ultimaRecepcion = ahora;
    }
    paquetes = paquetesRecibidos;
  }
  const bool activo = recibido && (ahora - ultimaRecepcion < TIEMPO_SIN_DMX_MS);
  digitalWrite(LED_BUILTIN, activo && valor >= 128 ? HIGH : LOW);
  if (ahora - ultimoInforme >= 1000) {
    ultimoInforme = ahora;
    Serial.print(activo ? F("DMX activo; canal 1=") : F("Sin DMX; ultimo valor="));
    Serial.print(valor);
    Serial.print(F("; paquetes="));
    Serial.println(paquetes);
  }
}
