#include <Arduino.h>
#include <Wire.h>

// Buscar dispositivos I2C en SDA (20) y SCL (21) del Mega.
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setWireTimeout(25000, true);
}

void loop() {
  uint8_t encontrados = 0;
  Serial.println(F("Explorando bus I2C..."));
  for (uint8_t direccion = 8; direccion < 120; ++direccion) {
    Wire.beginTransmission(direccion);
    const uint8_t resultado = Wire.endTransmission();
    if (resultado == 0) {
      Serial.print(F("Dispositivo en 0x"));
      if (direccion < 16) Serial.print('0');
      Serial.println(direccion, HEX);
      ++encontrados;
    }
    if (Wire.getWireTimeoutFlag()) {
      Serial.println(F("Tiempo agotado: comprobar SDA y SCL."));
      Wire.clearWireTimeoutFlag();
      break;
    }
  }
  Serial.print(F("Dispositivos encontrados: "));
  Serial.println(encontrados);
  delay(3000);
}
