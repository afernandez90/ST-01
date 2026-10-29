#pragma once
#include <stdint.h>
// Identidad fija de la unidad; el número de dispositivo se expresa en hexadecimal.
#define ST01_UID_FABRICANTE 0x7FF0
#define ST01_UID_DISPOSITIVO 0x66601337UL
#define ST01_UID_BYTES { uint8_t(ST01_UID_FABRICANTE >> 8), uint8_t(ST01_UID_FABRICANTE), uint8_t(ST01_UID_DISPOSITIVO >> 24), uint8_t(ST01_UID_DISPOSITIVO >> 16), uint8_t(ST01_UID_DISPOSITIVO >> 8), uint8_t(ST01_UID_DISPOSITIVO) }
