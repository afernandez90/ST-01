#pragma once
#include <stdint.h>
#include <string.h>
#include "version.h"
#include "identidad.h"

// Respondedor del prototipo: sin dependencias de Arduino ni escrituras en EEPROM.
// El identificador 7FF0 está reservado para experimentación, no para distribución.
class RespondedorRdm {
 public:
  const char *etiquetaVersion = ST01_VERSION " RDM prototipo";
  uint16_t (*gestorFtc)(uint16_t,bool,const uint8_t*,uint8_t,uint8_t*,uint8_t&) = nullptr;
  uint8_t uid[6] = ST01_UID_BYTES;
  volatile uint16_t direccion = 1;
  volatile bool identificar = false;
  bool silenciado = false;
  volatile bool guardar = false;
  volatile bool cambioDireccion = false;
  volatile bool cambioIdentificacion = false;
  char etiqueta[33] = "ST-01";
  uint8_t salida[257];
  uint16_t longitudSalida = 0;
  bool descubrimiento = false;

  static uint16_t leer16(const uint8_t *p) { return (uint16_t(p[0]) << 8) | p[1]; }
  static void escribir16(uint8_t *p, uint16_t n) { p[0] = n >> 8; p[1] = n; }

  bool procesar(const uint8_t *p, uint16_t n) {
    longitudSalida = 0;
    descubrimiento = false;
    if (n < 26 || n > 257 || p[0] != 0xcc || p[1] != 1 ||
        p[2] < 24 || n != uint16_t(p[2]) + 2 || p[23] != p[2] - 24) return false;
    uint16_t suma = 0;
    for (uint16_t i = 0; i < n - 2; ++i) suma += p[i];
    if (suma != leer16(p + n - 2)) return false;
    const bool todos = p[3] == 0xff && p[4] == 0xff;
    const bool fabricante = p[3] == uid[0] && p[4] == uid[1];
    const bool difusion = (todos || fabricante) &&
        p[5] == 0xff && p[6] == 0xff && p[7] == 0xff && p[8] == 0xff;
    if (!difusion && memcmp(p + 3, uid, 6)) return false;
    const uint8_t clase = p[20], datos = p[23];
    const uint16_t pid = leer16(p + 21), subdispositivo = leer16(p + 18);
    if (clase != 0x10 && clase != 0x20 && clase != 0x30) return false;
    if (difusion && clase == 0x20) return false;

    // Cabecera de respuesta: conservar transacción y devolverla al controlador.
    memcpy(salida, p, 24);
    memcpy(salida + 3, p + 9, 6);
    memcpy(salida + 9, uid, 6);
    salida[16] = 0;
    salida[17] = 0;
    salida[20] = clase + 1;
    uint8_t longitud = 0;
    uint16_t error = 0xffff;
    if (subdispositivo != 0 && !(subdispositivo == 0xffff && clase == 0x30)) {
      error = 0x0009;  // Subdispositivo inexistente.
    } else if (clase == 0x10) {
      if (subdispositivo != 0) return false;
      if (pid == 1) {
        if (datos != 12 || silenciado || memcmp(uid, p + 24, 6) < 0 ||
            memcmp(uid, p + 30, 6) > 0) return false;
        // La respuesta de descubrimiento no lleva break ni cabecera normal.
        memset(salida, 0xfe, 7);
        salida[7] = 0xaa;
        suma = 0;
        for (uint8_t i = 0; i < 6; ++i) {
          salida[8 + 2*i] = uid[i] | 0xaa;
          salida[9 + 2*i] = uid[i] | 0x55;
          suma += salida[8 + 2*i];
          suma += salida[9 + 2*i];
        }
        salida[20] = (suma >> 8) | 0xaa;
        salida[21] = (suma >> 8) | 0x55;
        salida[22] = suma | 0xaa;
        salida[23] = suma | 0x55;
        longitudSalida = 24;
        descubrimiento = true;
        return true;
      }
      if ((pid != 2 && pid != 3) || datos != 0) return false;
      silenciado = pid == 2;
      salida[24] = salida[25] = 0;  // Campo de control: equipo raíz sin proxy.
      longitud = 2;
    } else {
      const bool escritura = clase == 0x30;
      switch (pid) {
        case 0x0050: {  // SUPPORTED_PARAMETERS; solo parámetros adicionales.
          const uint16_t adicionales[] = {0x0080, 0x0081, 0x00e0, 0x00e1};
          if (escritura) { error = 5; break; }
          if (datos) { error = 1; break; }
          for (uint8_t i = 0; i < 4; ++i) escribir16(salida + 24 + i*2, adicionales[i]);
          longitud = 8;
          if (gestorFtc) for(uint16_t pidFtc=0x1200;pidFtc<=0x1203;++pidFtc){
            escribir16(salida+24+longitud,pidFtc);longitud+=2;
          }
          break;
        }
        case 0x0060:  // DEVICE_INFO: E1.20, modelo 1, categoría genérica, versión común de ST-01.
          if (escritura) { error = 5; break; }
          if (datos) { error = 1; break; }
          memset(salida + 24, 0, 19);
          escribir16(salida + 24, 0x0100);
          escribir16(salida + 26, 1);
          salida[31] = ST01_VERSION_MAYOR;
          salida[32] = ST01_VERSION_MENOR;
          salida[33] = ST01_VERSION_PARCHE;
          escribir16(salida + 34, 19);
          salida[36] = salida[37] = 1;
          escribir16(salida + 38, direccion);
          longitud = 19;
          break;
        case 0x0080: case 0x0081: case 0x00c0: {
          if (escritura) { error = 5; break; }
          if (datos) { error = 1; break; }
          const char *texto = pid == 0x0080 ? "ST-01 " ST01_PERSONALIDAD :
                              pid == 0x0081 ? "Salto Temporal" : etiquetaVersion;
          longitud = strlen(texto);
          memcpy(salida + 24, texto, longitud);
          break;
        }
        case 0x0082:  // DEVICE_LABEL.
          if (escritura) {
            if (datos > 32) { error = 1; break; }
            memcpy(etiqueta, p + 24, datos);
            etiqueta[datos] = 0;
            guardar = true;
          } else {
            if (datos) { error = 1; break; }
            longitud = strlen(etiqueta);
            memcpy(salida + 24, etiqueta, longitud);
          }
          break;
        case 0x00e0:  // DMX_PERSONALITY: una única personalidad de 19 canales.
          if (datos != (escritura ? 1 : 0)) { error = 1; break; }
          if (escritura) { if (p[24] != 1) error = 6; }
          else { salida[24] = salida[25] = 1; longitud = 2; }
          break;
        case 0x00e1:  // DMX_PERSONALITY_DESCRIPTION.
          if (escritura) { error = 5; break; }
          if (datos != 1) { error = 1; break; }
          if (p[24] != 1) { error = 6; break; }
          salida[24] = 1;
          escribir16(salida + 25, 19);
          memcpy(salida + 27, ST01_PERSONALIDAD, sizeof(ST01_PERSONALIDAD)-1);
          longitud = 3 + sizeof(ST01_PERSONALIDAD)-1;
          break;
        case 0x00f0:  // DMX_START_ADDRESS.
          if (datos != (escritura ? 2 : 0)) { error = 1; break; }
          if (escritura) {
            const uint16_t nueva = leer16(p + 24);
            if (nueva < 1 || nueva > 494) { error = 6; break; }
            cambioDireccion = cambioDireccion || direccion != nueva;
            direccion = nueva;
            guardar = true;
          } else { escribir16(salida + 24, direccion); longitud = 2; }
          break;
        case 0x1000:  // IDENTIFY_DEVICE.
          if (datos != (escritura ? 1 : 0)) { error = 1; break; }
          if (escritura) {
            if (p[24] > 1) { error = 6; break; }
            cambioIdentificacion = cambioIdentificacion || identificar != bool(p[24]);
            identificar = p[24];
          } else { salida[24] = identificar; longitud = 1; }
          break;
        default:
          // FTC solo se acepta por UID explícito: no se anuncia difusión.
          if (difusion) return false;
          if (subdispositivo != 0) { error = 9; break; }
          error = gestorFtc ? gestorFtc(pid,escritura,p+24,datos,salida+24,longitud) : 0;
      }
    }
    if (difusion) return false;  // Aplicar SET/difusión, pero nunca contestarlo.
    if (error != 0xffff) {
      salida[16] = 2;
      escribir16(salida + 24, error);
      longitud = 2;
    }
    salida[23] = longitud;
    salida[2] = 24 + longitud;
    longitudSalida = uint16_t(salida[2]) + 2;
    suma = 0;
    for (uint16_t i = 0; i < longitudSalida - 2; ++i) suma += salida[i];
    escribir16(salida + longitudSalida - 2, suma);
    return true;
  }
};
