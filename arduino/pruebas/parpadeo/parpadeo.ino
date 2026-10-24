// Prueba visual para confirmar la carga en Arduino Mega 2560.
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Dos destellos rápidos, seguidos de una pausa larga.
  digitalWrite(LED_BUILTIN, HIGH);
  delay(150);
  digitalWrite(LED_BUILTIN, LOW);
  delay(400);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(350);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1200);
}
