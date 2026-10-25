# Continuidad del proyecto

Actualizado: 25 de octubre de 2026.

## Efectos LED v0.0.6

Se han implementado las 24 animaciones de Barra y Tira y el progreso manual de 60 pasos de Barra. Firmware, fixture y espacio de trabajo usan v0.0.6, con los mismos 19 canales. Véase [mapa de efectos](EFECTOS_LED.md). Validación física y carga pendientes; las menciones de v0.0.5 siguientes describen la validación anterior.

## Próximo paso acordado

El usuario comprará un **segundo Arduino Mega** y retomaremos la instalación y las pruebas del cargador cuando lo reciba. El segundo Mega actuará como programador con ArduinoISP; el Mega del proyecto será el objetivo.

El condensador elegido es **E-Projects B-0001-K10, electrolítico radial de 10 µF, 25 V, 105 °C**. El usuario ya dispone de cables puente. Después de cargar ArduinoISP, conectar el condensador al Mega programador: positivo a RESET y negativo (lado marcado con franja) a GND. No colocarlo en el Mega objetivo.

Antes de instalar, concretar el cableado ISP, leer y guardar flash/EEPROM/fusibles/bloqueos reales, y preparar la imagen de instalación conservando la aplicación y los parámetros. Después verificar carga USB, actualizaciones FTC por RDM, identificación de todas las salidas y recuperación tras interrupciones. El cargador está compilado y probado en simulación, pero todavía no está instalado ni validado físicamente. Véase `arduino/arranque_rdm/LEEME.md` y `docs/ACTUALIZACION_RDM.md`.

La prueba física de las tiras sigue pendiente: comprobar protocolo, orden de color y cantidad real de píxeles antes de fijar el montaje. Los pines y las cantidades provisionales elegidas están en `docs/CABLEADO_PROPUESTO.md`. No confundir una decisión de diseño con cableado ya validado.

## Documentación final del hardware pendiente

Cuando estén definidos y probados los componentes y el cableado, preparar la documentación del montaje definitivo:

- Lista completa de componentes electrónicos y eléctricos (BOM), con referencias, cantidades y especificaciones.
- Esquema eléctrico completo: alimentación, protecciones, distribución de masas, señales y circuitos de salida.
- Plano de cableado y recorrido físico de los cables, con pines, conectores y correspondencia entre las caras A y B.

Este entregable queda acordado para el cierre del diseño; documentar el montaje real validado sin dar por definitivas las decisiones todavía pendientes del prototipo.

## Pilotos y módulos de salida

Los pilotos son indicadores LED de panel **AD16-22DS de 22 mm, variante de 12 V**, confirmada por el usuario. Se alimentarán desde la distribución de 12 V, no desde las salidas del Mega. Los módulos adquiridos son JZK de doble MOSFET, anunciados para 5–36 V y control PWM; el fabricante anuncia disparo de 3,3–20 V, compatible con la salida de 5 V del Mega; quedan pendientes de comprobar el conexionado y la respuesta de los pilotos a la regulación PWM antes de fijar el circuito definitivo.

## Pruebas ya confirmadas

- Recepción DMX desde QLC+ mediante una interfaz DMX y módulo RS-485 hacia RX1, pin 19 del Mega.
- Regulación PWM del LED L integrado con el piloto ámbar B (desplazamiento 13, canal absoluto `direccionBase + 12`) y apagado tras un segundo sin datos.
- LCD de 16×2 en I2C 0x27: primera fila `ST-01 DMX: 001` con dirección pendiente y asterisco si hay cambios sin guardar; segunda fila con DMX OK/SIN DMX y `LNB` (LED Naranja Cara B) seguido del valor recibido, sin número de canal.
- Cinco botones con KCOM a GND e INPUT_PULLUP en pines 22–26. La dirección se edita sin afectar la recepción hasta Guardar, que aplica y almacena en EEPROM.
- El firmware asigna, de izquierda a derecha y por pin creciente, 22: −10, 23: −1, 24: +1, 25: +10 y 26: Guardar. El usuario observó una discrepancia entre botón físico y pin; queda pendiente confirmar el cableado físico.

El piloto azul adicional de Cara B es un indicador de estado: encendido con DMX OK y apagado con SIN DMX, independiente de las intensidades DMX. No pertenece a los siete pilotos DMX. Se ha implementado su salida en pin 9; falta completar y validar el montaje. Identificar la hace parpadear junto con el resto.

El espacio de trabajo debe usar el fixture **Salto Temporal / ST-01**, modo `v0.0.6`, no un fixture genérico. El maestro visible en la consola es de tipo Grand Master y no tiene un canal de dispositivo asignado.

La personalidad vigente es `Control v0.0.6`: en los canales de dígito, 0 apaga todos los segmentos y el punto decimal, 1–9 muestran 1–9 y 10 muestra 0. Los valores 11–20 muestran 0–9 con punto decimal, 21–27 los segmentos A–G y 28–34 esos segmentos con punto decimal. Esta codificación sustituye la de v0.0.1; actualizar los valores de escenas externas antes de usarlas. El dispositivo ocupa 19 canales, sin regulador maestro propio. QLC+ escala o limita los canales de intensidad; los dígitos y modos no deben escalarse. Para un apagado total que incluya los dígitos, usar blackout de todos los canales. La dirección base válida es 1–494. Las escenas de versiones anteriores deben desplazarse un canal al eliminar el antiguo maestro. Se ha implementado una salida de cuatro dígitos mediante TPIC6B595 encadenados; instalación y validación físicas pendientes, según el cableado propuesto.

## Estado de RDM

Pruebas del 25 de octubre de 2026 con un controlador y enlace bidireccional DMX/RDM. El firmware ST-01 no depende del fabricante ni del modelo del emisor.

- Se comprobó la salida DMX a 40 paquetes/s y el transporte RDM de la interfaz de pruebas. Esta configuración describe la prueba realizada, no un requisito de marca para ST-01.
- QLC+ **5.3.0** incluye transporte RDM en el complemento DMX USB, pero no se ha encontrado un flujo completo de descubrimiento y gestión RDM en su interfaz QML. No confundir las funciones del complemento ni la interfaz antigua de `ui/src/rdmmanager.*` con un gestor disponible en la interfaz actual. Usar una aplicación específica para las futuras pruebas RDM.
- Se ha añadido un respondedor RDM de prototipo a `recepcion_dmx`: descubrimiento, identificación, información, etiqueta y dirección remota persistente. La revisión 0.0.5 se ha cargado por USB y se ha validado físicamente el RDM básico: descubrimiento/silencio, información, identificación GET/SET, dirección/etiqueta y persistencia tras reinicio; configuración restaurada a dirección 1 y etiqueta ST-01. Detalles en [prueba física RDM](PRUEBAS_RDM.md).
- Adaptador confirmado por el usuario: **DollaTek B07DJ4TGY3**, anunciado con dirección automática. TX1 (pin 18) está conectado a su entrada TTL de transmisión, confirmado por el usuario; RX1 (19) se conserva. Se han recibido respuestas RDM normales y de descubrimiento. Verificar las etiquetas y el sentido real de TXD/RXD antes de conectar. La temporización de liberación del bus y la transmisión del break del adaptador siguen pendientes de medida. No dar por supuesto aislamiento galvánico.
- El usuario delegó la elección de pines y controladores. Se añadieron siete salidas PWM (2–8), estado (9), Barra WS2812B (10), Tira WS2811 (11) y cuatro TPIC6B595 para los números (48/49/51/52). Identificar hace parpadear todas esas salidas, el LED L y la iluminación del LCD. Cantidades de diseño provisionales: 60/300 píxeles; confirmar los grupos reales. Detalles y límites de temporización en [cableado propuesto](CABLEADO_PROPUESTO.md).
- Se ha implementado FTC 1.00 según E1.37-4-2026, cargador Mega de 8 KiB y herramienta de carga. Todo compilado y probado en simulación; no instalado ni probado físicamente. El usuario confirma que no tiene programador ISP: esa instalación inicial queda pendiente. El sketch no anuncia FTC con el cargador original. La gestión USB del nuevo cargador y los controladores FTC externos siguen pendientes de validación; no asumir compatibilidad con los PIDs de borradores. Véase [actualización RDM](ACTUALIZACION_RDM.md).
- UID experimental del único prototipo: `7FF0:66601337`; asignar un identificador de dispositivo diferente antes de conectar otro ejemplar. El número 66601337 es hexadecimal, elegido por el usuario y compartido desde `identidad.h`; no proviene del hardware ni de la dirección DMX. Instrucciones y límites en [RDM](RDM.md).

## Maqueta 3D provisional

El espacio de trabajo QLC+ incluye `qlcplus/models/ST-01-cubo.obj`, con materiales, luces y dígitos estáticos en dos caras adyacentes de un cubo. Las dimensiones se han estimado por petición del usuario; no usarlas para fabricación. La maqueta no sigue DMX. Las instrucciones y el generador están en `qlcplus/models/LEEME.md`.

## Entorno de desarrollo

La configuración del entorno de desarrollo está completada. Se utilizan Arduino CLI 1.5.1, Arduino AVR Boards 1.8.8, hd44780 1.3.2 y FastLED 3.9.20. Las pruebas `parpadeo`, `escaner_i2c` y `recepcion_dmx` compilan para `arduino:avr:mega:cpu=atmega2560`. Se han realizado cargas por USB y comprobado la recepción DMX a unos 40 paquetes/s, con LCD en 0x27.

El proyecto de QLC+ 5.3.0 está en `qlcplus/SaltoTemporal.qxw` y la definición del fixture en `qlcplus/fixtures/ST-01.qxf`. Configure la salida del universo según la interfaz o red disponible.

Los archivos locales de VS Code están ignorados. Las tareas de carga y monitor deben detectar el puerto del Mega mediante Arduino CLI; no fijar nombres de puerto de un equipo concreto. Si no hay ningún Mega o hay varios, detenerse con un aviso.

Conservar los metadatos Creator originales hasta que QLC+ vuelva a guardar los archivos; no sustituirlos manualmente por una versión que no los haya guardado.

## Repositorio y preferencias

Utilizar el repositorio de trabajo activo como fuente del proyecto. Las rutas de los documentos son relativas a la raíz del repositorio.

Documentación y comentarios en español; conservar el nombre README.md en la raíz. Los archivos de VS Code son locales y están ignorados. Crear commits locales de los cambios terminados y subir solo en hitos o cuando el usuario lo solicite. Al unificar cambios nuevos, conservar los hitos publicados salvo petición expresa de reescribir todo el historial.

## Versión compartida

Por petición del usuario, firmware reportado y personalidad de QLC+ se alinean ahora en **0.0.6**. La revisión añade efectos a los selectores RGB y conserva los 19 canales y la codificación de los dígitos. Los textos y el identificador numérico RDM se generan desde `arduino/pruebas/recepcion_dmx/version.h`; al publicar una versión, actualizar también el modo de QLC+, la instancia del espacio de trabajo y la documentación. No incrementar el parche por cada edición.

La herramienta de pruebas obtiene el UID del controlador mediante la API de su transporte. El Arduino responde al UID fuente de cada solicitud; no lleva un controlador de destino fijado. Tras las pruebas RDM se comprobó también DMX a unos 39–40 paquetes/s y apagado por ausencia de señal.
