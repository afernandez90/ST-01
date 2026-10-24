# Continuidad del proyecto

Actualizado: 24 de octubre de 2026.

## Próximo paso acordado

Cuando llegue la fuente de alimentación, conectar y probar la **Barra LED de Cara A**, de 5 V y tipo ARGB/WS2812B compatible. Antes de alimentar, comprobar la salida real de la fuente y la polaridad del conector. Alimentar la barra directamente desde la fuente, compartir GND con el Mega y mantener el Mega alimentado por USB durante las pruebas; no unir la salida externa de 5 V al pin 5V del Mega.

Primero ejecutar una prueba con brillo limitado para confirmar protocolo, orden de color y número de píxeles. Después integrar el regulador maestro y los desplazamientos DMX 6–8 (RGB) y 9 (modo). Los efectos se generan en el Arduino. La cantidad de píxeles todavía no está confirmada.

## Pruebas ya confirmadas

- Recepción DMX desde QLC+ mediante ENTTEC DMX USB Pro y módulo RS-485 hacia RX1, pin 19 del Mega.
- Regulación PWM del LED integrado con el primer canal de la dirección activa y apagado tras un segundo sin datos.
- LCD de 16×2 en I2C 0x27: dirección, DMX OK/SIN DMX y valor MST.
- Cinco botones con KCOM a GND e INPUT_PULLUP en pines 22–26. La dirección se edita sin afectar la recepción hasta Guardar, que aplica y almacena en EEPROM.
- El firmware asigna 22: −1, 23: +1, 24: −10, 25: +10 y 26: Guardar. El usuario observó una discrepancia entre botón físico y pin; queda pendiente confirmar el cableado físico.

El piloto azul adicional de Cara B es un indicador de estado: encendido con DMX OK y apagado con SIN DMX, independiente del regulador maestro. No pertenece a los siete pilotos controlados por DMX. Su salida física todavía no está implementada.

La definición de fixture y el espacio de trabajo QLC+ incluyen el valor 34 para apagar todos los segmentos y el punto decimal de cada dígito. La salida física de los dígitos todavía no está implementada.

## Cambio futuro a MacBook Pro

El usuario quiere trasladar el desarrollo a su MacBook Pro más adelante. No realizar esta migración todavía.

Cuando lo solicite, abrir o clonar el repositorio en el Mac y configurar ChatGPT/Codex para trabajo local, VS Code, Arduino CLI, Arduino AVR Boards y hd44780. Recrear los archivos de IDE ignorados con las rutas y el puerto serie del Mac; no reutilizar las rutas absolutas de Linux. Instalar QLC+, importar el fixture y el espacio de trabajo, y comprobar de nuevo la salida ENTTEC. El firmware y el cableado no requieren cambios por el sistema operativo.

## Repositorio y preferencias

Repositorio de trabajo actual: `/home/alejandro/repos/ST-01`. La carpeta `/home/alejandro/Documents/ChatGPT/ST-01` contiene una copia anterior y no debe usarse como fuente actual.

Documentación y comentarios en español; conservar el nombre README.md en la raíz. Los archivos de VS Code son locales y están ignorados. Crear commits locales de los cambios terminados y subir solo en hitos o cuando el usuario lo solicite. Al unificar cambios nuevos, conservar los hitos publicados salvo petición expresa de reescribir todo el historial.
