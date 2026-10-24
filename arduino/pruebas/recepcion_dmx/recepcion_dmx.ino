#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// Entrada DMX por RX1 (pin 19). No utilizar Serial1 con esta interrupción.
constexpr uint16_t NUMERO_CANALES = 20;
constexpr uint16_t DIRECCION_MAXIMA = 513 - NUMERO_CANALES;
volatile uint16_t direccionDmx = 1;
uint16_t direccionPendiente = 1;
constexpr uint16_t MARCA_CONFIGURACION = 0x5301;
struct Configuracion {
  uint16_t marca;
  uint16_t direccion;
};

// KCOM a GND; las resistencias internas mantienen las entradas en HIGH.
constexpr uint8_t PINES_BOTONES[] = {22, 23, 24, 25, 26};
constexpr int8_t PASOS_DIRECCION[] = {-1, 1, -10, 10};
struct EstadoBoton {
  bool lectura;
  bool estable;
  unsigned long ultimoCambio;
};
EstadoBoton botones[5];
constexpr unsigned long TIEMPO_SIN_DMX_MS = 1000;
// Dirección del adaptador PCF8574 comprobada con el escáner I2C.
hd44780_I2Cexp pantalla(0x27);
bool pantallaDisponible = false;

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
  if (canal == direccionDmx) {
    valorRecibido = dato;
    datoPendiente = true;
    ++paquetesRecibidos;
  }
  if (canal >= 512) paqueteValido = false;
}

// Devuelve true solo al aplicar una nueva dirección con Guardar.
bool procesarBotones(unsigned long ahora) {
  bool modificada = false;
  for (uint8_t i = 0; i < 5; ++i) {
    const bool lectura = digitalRead(PINES_BOTONES[i]);
    EstadoBoton &boton = botones[i];
    if (lectura != boton.lectura) {
      boton.lectura = lectura;
      boton.ultimoCambio = ahora;
    }
    if (ahora - boton.ultimoCambio < 30 || lectura == boton.estable) continue;
    boton.estable = lectura;
    if (lectura != LOW) continue;
    if (i == 4) {
      // EEPROM.put solo escribe los bytes que cambian.
      const Configuracion configuracion = {MARCA_CONFIGURACION, direccionPendiente};
      EEPROM.put(0, configuracion);
      if (direccionPendiente != direccionDmx) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          direccionDmx = direccionPendiente;
          datoPendiente = false;
        }
        modificada = true;
      }
      Serial.println(F("Direccion guardada en EEPROM."));
    } else {
      const uint16_t nueva = constrain(static_cast<int>(direccionPendiente) + PASOS_DIRECCION[i],
                                       1, static_cast<int>(DIRECCION_MAXIMA));
      direccionPendiente = nueva;
    }
  }
  return modificada;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  Configuracion configuracion;
  EEPROM.get(0, configuracion);
  if (configuracion.marca == MARCA_CONFIGURACION &&
      configuracion.direccion >= 1 && configuracion.direccion <= DIRECCION_MAXIMA) {
    direccionDmx = configuracion.direccion;
  }
  direccionPendiente = direccionDmx;
  for (uint8_t i = 0; i < 5; ++i) {
    pinMode(PINES_BOTONES[i], INPUT_PULLUP);
    const bool lectura = digitalRead(PINES_BOTONES[i]);
    botones[i] = {lectura, lectura, millis()};
  }
  Wire.begin();
  Wire.setWireTimeout(25000, true);
  const int estadoPantalla = pantalla.begin(16, 2);
  pantallaDisponible = (estadoPantalla == 0);
  Serial.print(F("Estado LCD (0=correcto): "));
  Serial.println(estadoPantalla);
  // 16 MHz / (16 * (3 + 1)) = 250000 baudios; formato 8N2.
  UCSR1B = 0;
  UCSR1A = 0;
  UBRR1H = 0;
  UBRR1L = 3;
  UCSR1C = _BV(USBS1) | _BV(UCSZ11) | _BV(UCSZ10);
  UCSR1B = _BV(RXEN1) | _BV(RXCIE1);
  Serial.println(F("Prueba DMX: direccion ajustable, RX1 pin 19, brillo PWM 0-255."));
}

void loop() {
  static unsigned long ultimaRecepcion = 0;
  static unsigned long ultimoInforme = 0;
  static unsigned long ultimaPantalla = 0;
  static bool recibido = false;
  static uint8_t valor = 0;
  uint16_t paquetes;
  const unsigned long ahora = millis();
  if (procesarBotones(ahora)) {
    recibido = false;
    valor = 0;
  }
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
  // El pin 13 del Mega admite PWM: aplicar el brillo recibido por DMX.
  analogWrite(LED_BUILTIN, activo ? valor : 0);
  // Actualizar sin borrar toda la pantalla para evitar parpadeos.
  if (pantallaDisponible && ahora - ultimaPantalla >= 250) {
    ultimaPantalla = ahora;
    char linea[17];
    // El asterisco indica cambios pendientes de guardar.
    snprintf(linea, sizeof(linea), "ST-01 DMX: %03u %c", direccionPendiente,
             direccionDmx != direccionPendiente ? '*' : ' ');
    pantalla.setCursor(0, 0);
    pantalla.print(linea);
    snprintf(linea, sizeof(linea), activo ? "DMX OK  MST:%3u " : "SIN DMX MST:%3u ", valor);
    pantalla.setCursor(0, 1);
    pantalla.print(linea);
  }
  if (ahora - ultimoInforme >= 1000) {
    ultimoInforme = ahora;
    Serial.print(F("Direccion="));
    Serial.print(direccionDmx);
    Serial.print(activo ? F("; DMX activo; valor=") : F("; sin DMX; ultimo valor="));
    Serial.print(valor);
    Serial.print(F("; paquetes="));
    Serial.println(paquetes);
  }
}
