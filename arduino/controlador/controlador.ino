#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "transporte_rdm.h"
#include "salidas.h"
#include "hardware.h"
#include "actualizacion_rdm.h"

// Entrada DMX por RX1 (pin 19). No utilizar Serial1 con esta interrupción.
constexpr uint16_t NUMERO_CANALES = 19;
// Desplazamiento DMX convencional (empieza en uno): piloto ámbar de Cara B.
constexpr uint16_t CH_LAMP_ORANGE_B = 13;
static_assert(CH_LAMP_ORANGE_B <= NUMERO_CANALES, "Canal fuera de la personalidad DMX");
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
constexpr int8_t PASOS_DIRECCION[] = {-10, -1, 1, 10};
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
// Dos destellos de 250 ms al iniciar, sin bloquear la recepción DMX/RDM.
unsigned long inicioAutocomprobacion = 0;
bool autocomprobacionPendiente = true;

volatile uint8_t valorRecibido = 0;
volatile uint8_t canalesRecibidos[CANALES_ST01] = {};
volatile bool datoPendiente = false;
volatile uint16_t paquetesVentana = 0;
volatile uint16_t bytesRxVentana=0, breaksVentana=0, iniciosRdmVentana=0;
volatile uint16_t tramasRdmVentana=0, respuestasRdmVentana=0, erroresUartVentana=0;
volatile bool paqueteRdm=false;
volatile unsigned long ultimoByteRdm=0;

ISR(USART1_RX_vect) {
  const uint8_t estado = UCSR1A;
  const uint8_t dato = UDR1;
  ++bytesRxVentana;
  static bool esperaInicio = false;
  static bool paqueteValido = false;
  static uint16_t canal = 0;

  // El break produce un error de trama y un byte cero.
  if (estado & _BV(FE1)) {
    ++breaksVentana;
    if(paqueteRdm)TrazaRdm::registrar(TrazaRdm::CorteBreak,entradaRdm,indiceRdm);
    esperaInicio = (dato == 0);
    paqueteValido = false;
    canal = 0;
    paqueteRdm = false;
    indiceRdm = 0;
    return;
  }
  if (estado & (_BV(DOR1) | _BV(UPE1))) {
    ++erroresUartVentana;
    if(paqueteRdm){
      if(indiceRdm<sizeof(entradaRdm))entradaRdm[indiceRdm++]=dato;
      TrazaRdm::registrar(TrazaRdm::ErrorUart,entradaRdm,indiceRdm,estado);
    }
    paqueteRdm = false;
    esperaInicio = false;
    paqueteValido = false;
    return;
  }
  if (esperaInicio) {
    esperaInicio = false;
    paqueteValido = (dato == 0);
    paqueteRdm = (dato == 0xcc);
    if (paqueteRdm) { ++iniciosRdmVentana; entradaRdm[0] = dato; indiceRdm = 1; ultimoByteRdm=millis(); }
    return;
  }
  if (paqueteRdm) {
    entradaRdm[indiceRdm++] = dato;
    ultimoByteRdm=millis();
    if (indiceRdm == 3 && (dato < 24 || entradaRdm[1] != 1)) {
      paqueteRdm=false;
      TrazaRdm::registrar(TrazaRdm::CabeceraInvalida,entradaRdm,indiceRdm);
    }
    if (paqueteRdm && indiceRdm >= 3 && indiceRdm == uint16_t(entradaRdm[2]) + 2) {
      paqueteRdm = false;
      ++tramasRdmVentana;
      const bool responder = rdm.procesar(entradaRdm, indiceRdm);
      TrazaRdm::registrar(TrazaRdm::Rx,entradaRdm,indiceRdm,(responder?1:0)|(rdm.silenciado?2:0));
      // La dirección se aplica antes del siguiente paquete DMX, incluso en difusión.
      if (direccionDmx != rdm.direccion) {
        direccionDmx = rdm.direccion;
        datoPendiente = false;
        paquetesVentana = 0;
      }
      if (responder) { ++respuestasRdmVentana; iniciarRespuestaRdm(); }
    }
    if (indiceRdm == sizeof(entradaRdm)) paqueteRdm = false;
    return;
  }
  if (!paqueteValido) return;
  ++canal;
  if (canal >= direccionDmx && canal < direccionDmx + NUMERO_CANALES)
    canalesRecibidos[canal - direccionDmx] = dato;
  if (canal == direccionDmx + CH_LAMP_ORANGE_B - 1) {
    valorRecibido = dato;
    datoPendiente = true;
    ++paquetesVentana;
  }
  if (canal >= 512) paqueteValido = false;
}

// Mantener la dirección con tres cifras, como en el LCD.
void imprimirDireccion(uint16_t direccion) {
  if (direccion < 100) Serial.print('0');
  if (direccion < 10) Serial.print('0');
  Serial.print(direccion);
}

void informarEstado(bool activo, uint8_t valor, unsigned long tasaDecimas) {
  Serial.print(activo ? F("DMX OK") : F("SIN DMX"));
  Serial.print(F(" | Piloto ambar B: "));
  Serial.print(valor);
  Serial.print(F(" | Paquetes/s: "));
  Serial.print(tasaDecimas / 10);
  Serial.print('.');
  Serial.print(tasaDecimas % 10);
  Serial.println();
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
        const uint16_t anterior = direccionDmx;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          direccionDmx = direccionPendiente;
          rdm.direccion = direccionPendiente;
          datoPendiente = false;
          paquetesVentana = 0;
        }
        Serial.print(F("[CONFIG] Direccion DMX cambiada: "));
        imprimirDireccion(anterior);
        Serial.print(F(" -> "));
        imprimirDireccion(direccionPendiente);
        Serial.println(F(". Guardada en EEPROM."));
        modificada = true;
      } else {
        Serial.print(F("[CONFIG] Direccion DMX guardada sin cambios: "));
        imprimirDireccion(direccionDmx);
        Serial.println();
      }
    } else {
      const uint16_t nueva = constrain(static_cast<int>(direccionPendiente) + PASOS_DIRECCION[i],
                                       1, static_cast<int>(DIRECCION_MAXIMA));
      if (nueva != direccionPendiente) {
        direccionPendiente = nueva;
        Serial.print(F("[CONFIG] Direccion pendiente: "));
        imprimirDireccion(direccionPendiente);
        Serial.println(F(". Pulsa Guardar para aplicarla."));
      }
    }
  }
  return modificada;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  iniciarSalidasHardware();
  Serial.begin(115200);
  Serial.println(F("[INICIO] ST-01. Diagnostico USB a 115200 baudios."));
  Configuracion configuracion;
  EEPROM.get(0, configuracion);
  if (configuracion.marca == MARCA_CONFIGURACION &&
      configuracion.direccion >= 1 && configuracion.direccion <= DIRECCION_MAXIMA) {
    direccionDmx = configuracion.direccion;
    Serial.println(F("[CONFIG] Direccion DMX recuperada de EEPROM."));
  } else {
    Serial.println(F("[CONFIG] EEPROM sin direccion valida; se utiliza 001."));
  }
  rdm.direccion = direccionDmx;
  actualizacion.disponible = detectarArranqueRdm();
  if(actualizacion.disponible)rdm.gestorFtc=gestionarFtc;
  // La etiqueta ocupa EEPROM 32–64, sin modificar la configuración existente.
  if (EEPROM.read(32) == 0x52) {
    for (uint8_t i = 0; i < 32; ++i) rdm.etiqueta[i] = EEPROM.read(33 + i);
    rdm.etiqueta[32] = 0;
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
  if (pantallaDisponible) {
    Serial.println(F("[LCD] Pantalla 16x2 disponible en I2C 0x27."));
  } else {
    Serial.print(F("[LCD] Error al iniciar la pantalla; codigo: "));
    Serial.println(estadoPantalla);
  }
  digitalWrite(18, HIGH);
  pinMode(18, OUTPUT);
  if (!DIRECCION_RS485_AUTOMATICA) {
    digitalWrite(PIN_DIRECCION_RS485, LOW);
    pinMode(PIN_DIRECCION_RS485, OUTPUT);
  }
  // 16 MHz / (16 * (3 + 1)) = 250000 baudios; formato 8N2.
  UCSR1B = 0;
  UCSR1A = 0;
  UBRR1H = 0;
  UBRR1L = 3;
  UCSR1C = _BV(USBS1) | _BV(UCSZ11) | _BV(UCSZ10);
  UCSR1B = _BV(RXEN1) | _BV(RXCIE1);
  Serial.print(F("[CONFIG] Direccion DMX: "));
  imprimirDireccion(direccionDmx);
  Serial.println(F(" | Recepcion: RX1, pin 19."));
  Serial.println(F("[CONFIG] LED L: piloto ambar de Cara B, desplazamiento DMX 13."));
  char uidTexto[14];
  snprintf(uidTexto,sizeof(uidTexto),"%04X:%08lX",ST01_UID_FABRICANTE,
           static_cast<unsigned long>(ST01_UID_DISPOSITIVO));
  Serial.print(F("[RDM] UID "));Serial.print(uidTexto);
  Serial.println(F(". Respuestas por TX1, pin 18."));
  Serial.println(DIRECCION_RS485_AUTOMATICA ? F("[RDM] Direccion RS-485 automatica; verificar retorno del adaptador.")
                                         : F("[RDM] DE y /RE controlados por pin 27."));
  Serial.println(actualizacion.disponible ? F("[FTC] Cargador RDM compatible disponible.")
                                         : F("[FTC] Falta instalar el cargador RDM mediante ISP."));
  informarEstado(false, 0, 0);
  inicioAutocomprobacion = millis();
  Serial.println(F("[INICIO] Autocomprobacion: dos destellos de todas las salidas."));
}

void loop() {
  static unsigned long ultimaRecepcion = 0;
  static unsigned long ultimoInforme = millis();
  static unsigned long ultimaPantalla = 0;
  static bool recibido = false;
  static uint8_t valor = 0;
  static uint8_t canales[CANALES_ST01] = {};
  static bool estadoAnterior = false;
  const unsigned long ahora = millis();
  comprobarReinicioRdm();
  // Cerrar una captura incompleta sin esperar al siguiente break.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    if(paqueteRdm && ahora-ultimoByteRdm>=100){
      TrazaRdm::registrar(TrazaRdm::TiempoAgotado,entradaRdm,indiceRdm);
      paqueteRdm=false;indiceRdm=0;
    }
  }
  TrazaRdm::procesar();
  bool direccionRemota = false, identificar = false, guardarRdm = false;
  bool cambioIdentificacion = false;
  uint16_t nuevaDireccion = 1;
  char etiqueta[33];
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    direccionRemota = rdm.cambioDireccion;
    rdm.cambioDireccion = false;
    guardarRdm = rdm.guardar;
    rdm.guardar = false;
    identificar = rdm.identificar;
    cambioIdentificacion = rdm.cambioIdentificacion;
    rdm.cambioIdentificacion = false;
    nuevaDireccion = rdm.direccion;
    if (guardarRdm) memcpy(etiqueta, rdm.etiqueta, sizeof(etiqueta));
  }
  // EEPROM y diagnóstico fuera de las interrupciones: no retrasar respuestas RDM.
  if (guardarRdm) {
    const Configuracion configuracion = {MARCA_CONFIGURACION, nuevaDireccion};
    EEPROM.put(0, configuracion);
    EEPROM.update(32, 0x52);
    for (uint8_t i = 0; i < 32; ++i) EEPROM.update(33 + i, etiqueta[i]);
    Serial.println(F("[RDM] Configuracion remota guardada en EEPROM."));
  }
  if (direccionRemota) {
    direccionPendiente = nuevaDireccion;
    Serial.print(F("[CONFIG] Direccion DMX cambiada por RDM: "));
    imprimirDireccion(nuevaDireccion);
    Serial.println();
  }
  if (cambioIdentificacion) Serial.println(identificar ? F("[RDM] Identificacion activada.")
                                                     : F("[RDM] Identificacion desactivada."));
  if (procesarBotones(ahora) || direccionRemota) {
    recibido = false;
    valor = 0;
    estadoAnterior = false;
    ultimoInforme = ahora;
  }
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (uint8_t i = 0; i < CANALES_ST01; ++i) canales[i] = canalesRecibidos[i];
    if (datoPendiente) {
      valor = valorRecibido;
      datoPendiente = false;
      recibido = true;
      ultimaRecepcion = ahora;
    }
  }
  const bool activo = recibido && (ahora - ultimaRecepcion < TIEMPO_SIN_DMX_MS);
  if (activo != estadoAnterior) {
    Serial.println(activo ? F("[DMX] Recepcion establecida o recuperada.")
                          : F("[DMX] Senal perdida: un segundo sin datos. Salida apagada."));
    estadoAnterior = activo;
  }
  const unsigned long tiempoAutocomprobacion = ahora - inicioAutocomprobacion;
  if (autocomprobacionPendiente && tiempoAutocomprobacion >= 1000) {
    autocomprobacionPendiente = false;
    Serial.println(F("[INICIO] Autocomprobacion terminada."));
  }
  const bool pruebaSalidas = identificar || autocomprobacionPendiente;
  // Reutilizar el patrón de identificación, empezando con un destello encendido.
  const unsigned long relojSalidas = autocomprobacionPendiente
      ? tiempoAutocomprobacion + 250 : ahora;
  const EstadoSalidas salidas = calcularSalidas(canales, activo, pruebaSalidas, relojSalidas);
  aplicarSalidasHardware(salidas);
  static bool iluminacionAnterior=true;
  const bool iluminacion=!pruebaSalidas || salidas.pilotoEstado;
  if(pantallaDisponible && iluminacion!=iluminacionAnterior){
    if(iluminacion)pantalla.backlight();else pantalla.noBacklight();
    iluminacionAnterior=iluminacion;
  }
  // Actualizar sin borrar toda la pantalla para evitar parpadeos.
  if (pantallaDisponible && ahora - ultimaPantalla >= 250) {
    ultimaPantalla = ahora;
    char linea[17];
    // El asterisco indica cambios pendientes de guardar.
    snprintf(linea, sizeof(linea), "ST-01 DMX: %03u %c", direccionPendiente,
             direccionDmx != direccionPendiente ? '*' : ' ');
    pantalla.setCursor(0, 0);
    if (autocomprobacionPendiente) snprintf(linea, sizeof(linea), "AUTOCOMPROBAR   ");
    else if (identificar) snprintf(linea, sizeof(linea), "IDENTIFICAR ST01");
    pantalla.print(linea);
    snprintf(linea, sizeof(linea), activo ? "DMX OK  LNB:%3u " : "SIN DMX LNB:%3u ", valor);
    pantalla.setCursor(0, 1);
    pantalla.print(linea);
  }
  const unsigned long intervalo = ahora - ultimoInforme;
  if (intervalo >= 1000) {
    uint16_t paquetes,bytesRx,breaks,iniciosRdm,tramasRdm,respuestasRdm,erroresUart,terminadas;
    // Contar solo los paquetes de esta ventana, sin acumulación ni desbordamiento del total.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      paquetes = paquetesVentana;
      paquetesVentana = 0;
      bytesRx=bytesRxVentana;bytesRxVentana=0;
      breaks=breaksVentana;breaksVentana=0;
      iniciosRdm=iniciosRdmVentana;iniciosRdmVentana=0;
      tramasRdm=tramasRdmVentana;tramasRdmVentana=0;
      respuestasRdm=respuestasRdmVentana;respuestasRdmVentana=0;
      erroresUart=erroresUartVentana;erroresUartVentana=0;
      terminadas=respuestasTerminadasVentana;respuestasTerminadasVentana=0;
    }
    if(iniciosRdm || erroresUart || (!activo && bytesRx)){
      Serial.print(F("[RDM] Bytes RX: "));Serial.print(bytesRx);
      Serial.print(F(" | Breaks: "));Serial.print(breaks);
      Serial.print(F(" | Inicios: "));Serial.print(iniciosRdm);
      Serial.print(F(" | Tramas completas: "));Serial.print(tramasRdm);
      Serial.print(F(" | Respuestas: "));Serial.print(respuestasRdm);
      Serial.print(F(" | TX terminadas: "));Serial.print(terminadas);
      Serial.print(F(" | Fase TX: "));Serial.print(static_cast<uint8_t>(faseRdm));
      Serial.print(F(" | Errores UART: "));Serial.println(erroresUart);
    }
    ultimoInforme = ahora;
    const unsigned long tasaDecimas = (paquetes * 10000UL + intervalo / 2) / intervalo;
    informarEstado(activo, valor, tasaDecimas);
  }
}
