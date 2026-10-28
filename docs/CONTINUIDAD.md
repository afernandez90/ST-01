# Continuidad del proyecto

Actualizado: 27 de octubre de 2026.

## Cola KITT, velocidades y referencias v0.0.9

Se conserva en `docs/datasheets/` una copia de las fichas CL5611AH (XLITX) y ELS-50101A/B (Etnal), enlazadas desde [las notas de circuito](REFERENCIAS_VISUALIZADORES.md), con patillajes, resistencias, polarización y condensadores propuestos.

KITT utiliza cabeza al 100 %, sin LED delantero, y diez LED traseros al 10, 9, …, 1 %; cabeza y cola suman once posiciones. La cola se invierte con la dirección y se recorta en los extremos. Los ciclos generales se aceleran a 3000–500 ms y el estrobo a 250–100 ms. Se aplica a Barra y Tira sin cambiar rangos de modo ni canales. Firmware, fixture y workspace usan v0.0.9; carga y validación física de esta revisión pendientes.

## Letras de los visualizadores v0.0.8

Se añaden A–Z en valores DMX 35–60 y a–z en 61–86 de los cuatro canales de dígito. Se conserva la codificación 0–34 y todos los rangos de efectos LED de v0.0.7; los valores 87–255 apagan el visualizador. Las letras son aproximaciones y algunas comparten segmentos. Véase [mapa de letras](LETRAS_VISUALIZADORES.md). La revisión v0.0.8 añadió estos códigos; siguen vigentes en v0.0.9. Su validación física sigue pendiente.

## Efectos LED v0.0.7

Se han implementado las 24 animaciones de Barra y Tira y el progreso manual de 38 pasos de Barra. La revisión v0.0.7 fijó los 38 píxeles de Barra y sus rangos, con los mismos 19 canales. Véase [mapa de efectos](EFECTOS_LED.md). El 26 de octubre de 2026 se cargó v0.0.6 por USB y el usuario comprobó RGB y 38 píxeles mediante progreso manual. La revisión v0.0.7 adapta el recuento y los rangos, se ha cargado y verificado por USB y su fixture se ha instalado en QLC+. Reiniciar QLC+ y abrir el espacio de trabajo actualizado para refrescar la personalidad y sus etiquetas. La validación de las demás animaciones sigue pendiente; las menciones de v0.0.5 siguientes describen la validación anterior.

## Próximo paso acordado

El usuario comprará un **segundo Arduino Mega** y retomaremos la instalación y las pruebas del cargador cuando lo reciba. El segundo Mega actuará como programador con ArduinoISP; el Mega del proyecto será el objetivo.

El condensador elegido es **E-Projects B-0001-K10, electrolítico radial de 10 µF, 25 V, 105 °C**. El usuario ya dispone de cables puente. Después de cargar ArduinoISP, conectar el condensador al Mega programador: positivo a RESET y negativo (lado marcado con franja) a GND. No colocarlo en el Mega objetivo.

El cargador se instaló por ISP el 27 de octubre de 2026, con respaldo y conservación de la aplicación 0.0.7, EEPROM y fusibles; flash, EEPROM y bloqueo verificados. Queda verificar carga USB, actualizaciones FTC por RDM, identificación de todas las salidas y recuperación tras interrupciones. Véase `arduino/arranque_rdm/LEEME.md` y `docs/ACTUALIZACION_RDM.md`.

La Barra WS2812B de 5 V tiene 38 píxeles comprobados físicamente; RGB y progreso manual funcionan. Completar la validación de sus animaciones. La Tira sigue pendiente de prueba: comprobar protocolo, orden de color y cantidad real de píxeles antes de fijar el montaje. Los pines y las cantidades provisionales elegidas están en `docs/CABLEADO_PROPUESTO.md`. No confundir una decisión de diseño con cableado ya validado.

## Documentación final del hardware pendiente

Cuando estén definidos y probados los componentes y el cableado, preparar la documentación del montaje definitivo:

- Lista completa de componentes electrónicos y eléctricos (BOM), con referencias, cantidades y especificaciones.
- Esquema eléctrico completo: alimentación, protecciones, distribución de masas, señales y circuitos de salida.
- Plano de cableado y recorrido físico de los cables, con pines, conectores y correspondencia entre las caras A y B.

Este entregable queda acordado para el cierre del diseño; documentar el montaje real validado sin dar por definitivas las decisiones todavía pendientes del prototipo.

## Visualizadores y pruebas por etapas

Los visualizadores previstos para el montaje son **50101AS**, rojos, de **5 pulgadas**, un dígito, ocho segmentos, **10 pines** y **cátodo común**; su llegada sigue pendiente. Se ha localizado la referencia Etnal ELS-50101AS, pero el fabricante de las unidades adquiridas no está confirmado; verificar sus características antes del montaje.

El usuario selecciona ahora **CL5611AH de 0,56 pulgadas** como visualizador pequeño de pruebas, sustituyendo la elección inicial 5101AS. Se han localizado fichas de XLITX y Etnal y se han comparado sus diagramas. Los patillajes de referencia difieren en los pines 1, 2 y 3; el grande también necesita mucha más tensión de segmento. Véanse [referencias, valores y patillajes](REFERENCIAS_VISUALIZADORES.md). Confirmar los componentes recibidos antes de concretar el montaje de prueba; no dar por comprada toda la lista sugerida.

La etapa con **74HC595 y TBD62783APG** es la candidata para probar el control de los segmentos. Cuando lleguen los 50101AS grandes, medir por separado patillaje, tensión y corriente de cada tipo de segmento y del punto decimal, y ajustar alimentación, resistencias y límites de la etapa de salida. Las medidas, resistencias y conexiones físicas del CL5611AH no se trasladan automáticamente al 50101AS. La lógica de dígitos y la personalidad DMX pueden conservarse.

La propuesta anterior con TPIC6B595 conectados directamente a los segmentos requiere ánodo común y no es compatible con estos visualizadores. Rediseñar la etapa de números antes del montaje y revisar su control en el firmware; la personalidad DMX puede conservarse. Véase [cableado propuesto](CABLEADO_PROPUESTO.md).

## Pilotos y módulos de salida

Los pilotos son indicadores LED de panel **AD16-22DS de 22 mm, variante de 12 V**, confirmada por el usuario. Se alimentarán desde la distribución de 12 V, no desde las salidas del Mega. Los módulos adquiridos son JZK de doble MOSFET, anunciados para 5–36 V y control PWM; el fabricante anuncia disparo de 3,3–20 V, compatible con la salida de 5 V del Mega; El 26 de octubre de 2026 el usuario confirma que el piloto rojo funciona correctamente con la fuente de 12 V y el módulo MOSFET, y acepta este montaje como patrón para los demás pilotos. El módulo presenta conexiones de potencia VIN/VOUT y control GND/TRIG/PWM. La asignación del piloto rojo es pin 5 del Mega, desplazamiento DMX 12. Conservar el conexionado físico que ha funcionado; no se ha documentado todavía la disposición exacta de sus terminales ni una medida de la curva de regulación PWM. Quedan pendientes el montaje y la prueba individual de los demás pilotos.

## Pruebas ya confirmadas

- Piloto rojo de 12 V y módulo MOSFET: funcionamiento confirmado por el usuario el 26 de octubre de 2026; patrón de montaje aceptado para los demás pilotos.
- Recepción DMX desde QLC+ mediante una interfaz DMX y módulo RS-485 hacia RX1, pin 19 del Mega.
- Regulación PWM del LED L integrado con el piloto ámbar B (desplazamiento 13, canal absoluto `direccionBase + 12`) y apagado tras un segundo sin datos.
- LCD de 16×2 en I2C 0x27: primera fila `ST-01 DMX: 001` con dirección pendiente y asterisco si hay cambios sin guardar; segunda fila con DMX OK/SIN DMX y `LNB` (LED Naranja Cara B) seguido del valor recibido, sin número de canal.
- Cinco botones con KCOM a GND e INPUT_PULLUP en pines 22–26. La dirección se edita sin afectar la recepción hasta Guardar, que aplica y almacena en EEPROM.
- El firmware asigna, de izquierda a derecha y por pin creciente, 22: −10, 23: −1, 24: +1, 25: +10 y 26: Guardar. El usuario observó una discrepancia entre botón físico y pin; queda pendiente confirmar el cableado físico.

El piloto azul adicional de Cara B es un indicador de estado: encendido con DMX OK y apagado con SIN DMX, independiente de las intensidades DMX. No pertenece a los siete pilotos DMX. Se ha implementado su salida en pin 9; falta completar y validar el montaje. Identificar la hace parpadear junto con el resto.

El espacio de trabajo debe usar el fixture **Salto Temporal / ST-01**, modo `v0.0.9`, no un fixture genérico. El maestro visible en la consola es de tipo Grand Master y no tiene un canal de dispositivo asignado.

La personalidad vigente es `Control v0.0.9`: en los canales de dígito, 0 apaga todos los segmentos y el punto decimal, 1–9 muestran 1–9 y 10 muestra 0. Los valores 11–20 muestran 0–9 con punto decimal, 21–27 los segmentos A–G y 28–34 esos segmentos con punto decimal. Esta codificación sustituye la de v0.0.1; actualizar los valores de escenas externas antes de usarlas. El dispositivo ocupa 19 canales, sin regulador maestro propio. QLC+ escala o limita los canales de intensidad; los dígitos y modos no deben escalarse. Para un apagado total que incluya los dígitos, usar blackout de todos los canales. La dirección base válida es 1–494. Las escenas de versiones anteriores deben desplazarse un canal al eliminar el antiguo maestro. Se ha implementado una salida de cuatro dígitos mediante TPIC6B595 encadenados; instalación y validación físicas pendientes, según el cableado propuesto.

## Estado de RDM

Pruebas del 25 de octubre de 2026 con un controlador y enlace bidireccional DMX/RDM. El firmware ST-01 no depende del fabricante ni del modelo del emisor.

- Se comprobó la salida DMX a 40 paquetes/s y el transporte RDM de la interfaz de pruebas. Esta configuración describe la prueba realizada, no un requisito de marca para ST-01.
- QLC+ **5.3.0** incluye transporte RDM en el complemento DMX USB, pero no se ha encontrado un flujo completo de descubrimiento y gestión RDM en su interfaz QML. No confundir las funciones del complemento ni la interfaz antigua de `ui/src/rdmmanager.*` con un gestor disponible en la interfaz actual. Usar una aplicación específica para las futuras pruebas RDM.
- Se ha añadido un respondedor RDM de prototipo a `controlador`: descubrimiento, identificación, información, etiqueta y dirección remota persistente. La revisión 0.0.5 se ha cargado por USB y se ha validado físicamente el RDM básico: descubrimiento/silencio, información, identificación GET/SET, dirección/etiqueta y persistencia tras reinicio; configuración restaurada a dirección 1 y etiqueta ST-01. Detalles en [prueba física RDM](PRUEBAS_RDM.md).
- Adaptador confirmado por el usuario: **DollaTek B07DJ4TGY3**, anunciado con dirección automática. TX1 (pin 18) está conectado a su entrada TTL de transmisión, confirmado por el usuario; RX1 (19) se conserva. Se han recibido respuestas RDM normales y de descubrimiento. Verificar las etiquetas y el sentido real de TXD/RXD antes de conectar. La temporización de liberación del bus y la transmisión del break del adaptador siguen pendientes de medida. No dar por supuesto aislamiento galvánico.
- El usuario delegó la elección de pines y controladores. Se añadieron siete salidas PWM (2–8), estado (9), Barra WS2812B (10), Tira WS2811 (11) y cuatro TPIC6B595 para los números (48/49/51/52). Identificar hace parpadear todas esas salidas, el LED L y la iluminación del LCD. Barra: 38 píxeles comprobados; Tira: 300 grupos provisionales, pendientes de contar. Detalles y límites de temporización en [cableado propuesto](CABLEADO_PROPUESTO.md).
- Se ha implementado FTC 1.00 según E1.37-4-2026, cargador Mega de 8 KiB y herramienta de carga. Compilado y probado en simulación; cargador instalado y verificado por ISP el 27 de octubre de 2026 mediante un segundo Mega con ArduinoISP. La actualización FTC física sigue pendiente. El sketch no anuncia FTC con el cargador original. La gestión USB del nuevo cargador y los controladores FTC externos siguen pendientes de validación; no asumir compatibilidad con los PIDs de borradores. Véase [actualización RDM](ACTUALIZACION_RDM.md).
- UID experimental del único prototipo: `7FF0:66601337`; asignar un identificador de dispositivo diferente antes de conectar otro ejemplar. El número 66601337 es hexadecimal, elegido por el usuario y compartido desde `identidad.h`; no proviene del hardware ni de la dirección DMX. Instrucciones y límites en [RDM](RDM.md).

## Maqueta 3D provisional

El espacio de trabajo QLC+ incluye `qlcplus/models/ST-01-cubo.obj`, con materiales, luces y dígitos estáticos en dos caras adyacentes de un cubo. Las dimensiones se han estimado por petición del usuario; no usarlas para fabricación. La maqueta no sigue DMX. Las instrucciones y el generador están en `qlcplus/models/LEEME.md`.

## Entorno de desarrollo

La configuración del entorno de desarrollo está completada. Se utilizan Arduino CLI 1.5.1, Arduino AVR Boards 1.8.8, hd44780 1.3.2 y FastLED 3.9.20. El firmware principal `arduino/controlador` y las pruebas `parpadeo` y `escaner_i2c` compilan para `arduino:avr:mega:cpu=atmega2560`. Se han realizado cargas por USB y comprobado la recepción DMX a unos 40 paquetes/s, con LCD en 0x27.

El proyecto de QLC+ 5.3.0 está en `qlcplus/SaltoTemporal.qxw` y la definición del fixture en `qlcplus/fixtures/ST-01.qxf`. Configure la salida del universo según la interfaz o red disponible.

La configuración activa `.vscode/` de la raíz está ignorada; las plantillas compartidas se versionan bajo `ide/`, organizadas por plataforma y con sus instrucciones de uso. Las tareas de carga y monitor deben detectar el puerto del Mega mediante Arduino CLI; no fijar nombres de puerto de un equipo concreto. Si no hay ningún Mega o hay varios, detenerse con un aviso.

Conservar los metadatos Creator originales hasta que QLC+ vuelva a guardar los archivos; no sustituirlos manualmente por una versión que no los haya guardado.

## Repositorio y preferencias

Utilizar el repositorio de trabajo activo como fuente del proyecto. Las rutas de los documentos son relativas a la raíz del repositorio.

Documentación y comentarios en español; conservar el nombre README.md en la raíz. Los archivos de VS Code son locales y están ignorados. Crear commits locales de los cambios terminados y subir solo en hitos o cuando el usuario lo solicite. Al unificar cambios nuevos, conservar los hitos publicados salvo petición expresa de reescribir todo el historial.

## Versión compartida

Por petición del usuario, firmware reportado y personalidad de QLC+ se alinean ahora en **0.0.9**. La revisión conserva las letras, los 19 canales y los rangos de modo, y cambia la cola KITT y las velocidades de los efectos. Los textos y el identificador numérico RDM se generan desde `arduino/controlador/version.h`; al publicar una versión, actualizar también el modo de QLC+, la instancia del espacio de trabajo y la documentación. No incrementar el parche por cada edición.

La herramienta de pruebas obtiene el UID del controlador mediante la API de su transporte. El Arduino responde al UID fuente de cada solicitud; no lleva un controlador de destino fijado. Tras las pruebas RDM se comprobó también DMX a unos 39–40 paquetes/s y apagado por ausencia de señal.

### Autocomprobación al iniciar

El controlador realiza dos destellos de todas las salidas durante el primer segundo después de inicializar el hardware, reutilizando el patrón de identificación RDM sin modificar IDENTIFY_DEVICE. La recepción DMX y las respuestas RDM continúan durante la prueba; al finalizar se aplica el DMX vigente o se apagan las salidas sin señal válida. Validado mediante simulación integrada y compilación para Mega; pendiente de cargar y comprobar físicamente.

### Preparación ISP — 27 de octubre de 2026

Segundo Mega preparado con el ejemplo oficial ArduinoISP y condensador de 100 µF en RESET. El controlador se identifica correctamente como ATmega2560 después de desconectar su cableado externo. Respaldos completos de flash/EEPROM/fusibles/bloqueos en `build/respaldos/mega-20261027/` (locales, no versionados). La instalación conserva la aplicación 0.0.7; la revisión 0.0.9 no se carga en este paso. Véase el [registro de instalación](INSTALACION_ARRANQUE_RDM.md) para resultados y validaciones pendientes.

La primera prueba USB confirmó arranque normal, detección FTC e identificación del chip, pero detectó comandos de lectura flash/escritura EEPROM intercambiados en STK500v2. Código corregido con prueba nativa del receptor; **cargador corregido reinstalado y verificado por ISP; pendiente repetir las pruebas USB**. No se hicieron escrituras USB.

La lectura EEPROM USB mostró además una discrepancia de direccionamiento respecto al respaldo ArduinoISP/STK500v1. Antes del próximo borrado, cotejar la lectura USB actual mediante SPI universal por dirección de byte y usar un respaldo/restauración verificados; no reutilizar a ciegas el primer `eeprom.hex`. Detalles en el registro de instalación.

Segunda instalación completada: EEPROM preservada mediante EESAVE temporal y comparada por SPI universal (4096 bytes idénticos antes/después). Fusibles originales restaurados y releídos, bloqueo activo. Aplicación conservada 0.0.7. Pendiente verificación USB de la corrección y transferencia FTC física.

La corrección ya está validada por USB: lectura/verificación de aplicación y EEPROM correctas, carga real de la misma 0.0.7 y verificación de 31 400 bytes, y arranque normal con dirección 001 y cargador FTC detectado. EEPROM conserva todos sus bytes salvo la marca de aplicación válida (96: FF→A5), cambio previsto. Próximo paso: reconectar el enlace RS-485 y validar actualización FTC física y recuperación.

Primera prueba FTC física: RDM y anuncio FTC correctos, pero timeout durante el reinicio al cargador, antes de enviar bloques. Desensamblado confirma que LTO eliminó la rutina temprana de desactivación watchdog. Corregido con atributos de retención y verificador del desensamblado (con prueba negativa); **pendiente reinstalar por ISP esta corrección y repetir FTC físico**.

Corrección watchdog instalada y verificada por ISP: 39 504 bytes de imagen; aplicación 0.0.7 conservada, EEPROM idéntica (4096 bytes), fusible original y bloqueo restaurados/verificados. Pendiente reconectar USB del controlador y enlace RDM para repetir la transferencia FTC física.

Actualización FTC física completada con 0.0.7: transferencia de 31 416 bytes, CRC/validación, commit y retorno a aplicación correctos. Pruebas RDM completas posteriores con parámetros restaurados. También validados CRC de bloque erróneo, duplicado, offset fuera de secuencia y recuperación tras cerrar/reabrir el enlace con 384 bytes transferidos, seguida de recarga completa. Pendiente corte eléctrico; véase [pruebas FTC](PRUEBAS_FTC.md).

Estado físico al terminar esta preparación: carga parcial FTC de 384 bytes, sesión 90, equipo dejado intencionadamente en el cargador para que el usuario corte/restablezca alimentación. Informe `build/respaldos/mega-20261027/preparacion-corte-ftc.json`. Tras el corte, comprobar respuesta del cargador y ejecutar carga completa de `ST-01-v0.0.7.st01`; después comprobar dirección 1 y etiqueta ST-01. No considerar completado el ensayo de corte hasta esa recuperación.

### Resultado final de la recuperación tras corte

El usuario cortó/restauró alimentación durante una carga parcial de 384 bytes. El equipo volvió en modo `ST01 cargador 1.0`, conservando dirección 1 y etiqueta ST-01. Una carga completa nueva por RDM de la misma 0.0.7 terminó con CRC/validación, commit y reinicio correctos. RDM posterior confirma descubrimiento/silencio, FTC disponible y mismos firmware, dirección, etiqueta y personalidad. **Ensayo de corte eléctrico completado; el controlador vuelve a funcionamiento normal.** No necesitó USB ni ISP para recuperar. Informes en el respaldo local, resumen en [pruebas FTC](PRUEBAS_FTC.md). La revisión 0.0.9 y la autocomprobación de arranque siguen pendientes de carga por decisión del usuario.

### Preferencia permanente de actualización

Por petición expresa del usuario, desde ahora todas las actualizaciones de la aplicación del controlador se realizan por RDM. No volver a usar carga USB como procedimiento habitual. `herramientas/cargar_rdm.sh` compila, empaqueta y carga por RDM; tarea VS Code **RDM: compilar y cargar controlador**, activa y versionada bajo `ide/mac/`. F5 y la carga del sketch activo también derivan a RDM cuando el sketch es `controlador`. Los sketches de prueba independientes conservan su carga USB; USB/ISP del controlador se reservan para diagnóstico/recuperación excepcional.

El usuario pidió conservar también una tarea USB de respaldo: **USB: cargar controlador (respaldo)**, mediante `ST01_ACTION=upload-usb`. Se utiliza solo al elegirla expresamente; el flujo habitual y F5 continúan por RDM. No se ejecuta esa carga USB para validar esta configuración.

### Firmware actual cargado por RDM

Por petición del usuario se compiló y cargó la última revisión **0.0.9 exclusivamente por RDM**, ejecutando `herramientas/cargar_rdm.sh`, el mismo lanzador de la tarea VS Code. Paquete de 32 398 bytes; validación/commit/reinicio correctos. Consulta RDM posterior confirma versión/modelo/personalidad 0.0.9, 19 canales, dirección 1, etiqueta ST-01, descubrimiento/silencio y FTC disponible. Incluye letras, KITT con cola larga, velocidades aumentadas y autocomprobación de dos destellos al arrancar; la comprobación visual de las salidas conectadas corresponde al usuario. Registro local `carga-v0.0.9-rdm.log` e informe `rdm-v0.0.9.json`. El estado actual sustituye las notas anteriores que indicaban 0.0.7 instalada o 0.0.9 pendiente de cargar.

Ajuste KITT del 28 de octubre de 2026: eliminado el LED delantero al 60 % y ampliado el desvanecimiento a veinte posiciones (cabeza 100 % y 19 detrás al 95–5 %, pasos de 5 %). Compartido por Barra y Tira, sin cambiar canales, modos ni velocidades.

Pruebas completas y compilación del ajuste KITT superadas. Paquete actual de 32 386 bytes, CRC FTC 9394. Carga RDM pendiente: la interfaz devuelve `Resource busy`; cerrar la aplicación que la ocupa antes de repetir `herramientas/cargar_rdm.sh`. El controlador conserva por ahora el KITT anterior.

El usuario pidió probar una curva exponencial inversa en la cola KITT: f(d)=0,05+0,95·(exp(3)−exp(3·d/19))/(exp(3)−1), d=0…19, tabla de enteros precalculada sin coma flotante en el Mega. Conserva veinte posiciones y frente oscuro. La pendiente aumenta hacia el final; es un ajuste visual experimental, sin afirmar calibración perceptual. Sustituye el ajuste lineal anterior, que aún no se había cargado por estar ocupada la interfaz RDM.

Curva exponencial inversa: suite completa y compilación correctas (paquete 32 380 bytes, CRC FTC BDA9). La interfaz sigue ocupada (`Resource busy`), por lo que la actualización RDM permanece pendiente y el controlador conserva la cola anterior. Registro `build/controlador/carga-kitt-exponencial-20261028.log`.

Tras cerrar la aplicación que ocupaba la interfaz, la curva KITT exponencial inversa se cargó correctamente por RDM: 32 380 bytes, CRC FTC BDA9, validación/commit y reinicio completados; versión reportada 0.0.9. El controlador ya incorpora frente oscuro y cola extendida de veinte posiciones con la nueva curva; pendiente valoración visual del usuario.

KITT: curvatura exponencial inversa aumentada de k=3 a k=6 por petición del usuario. Mantiene veinte posiciones y frente oscuro; tabla limitada a 255−d para asegurar una única cabeza y descenso estricto con resolución de 8 bits. Suite completa superada y carga RDM finalizada con validación/commit/reinicio; versión 0.0.9 confirmada. Registro `build/controlador/carga-kitt-k6-20261028.log`.

KITT ajustado nuevamente por petición del usuario: cabeza 100 % y diez píxeles traseros al 10, 9, …, 1 %, sin luz por delante. Sustituye las curvas exponenciales experimentales y conserva el escalado RGB, velocidades y rangos DMX.

La cola tenue KITT (100 % de cabeza, 10–1 % detrás) supera la suite completa y se ha cargado correctamente por RDM con validación/commit/reinicio y versión 0.0.9 confirmada. Registro `build/controlador/carga-kitt-cola-tenue-20261028.log`.

Estrobo: duplicada la velocidad de cada modo en Barra y Tira (periodos previos divididos entre dos, 125–50 ms / 8–20 Hz). Refresco permitido cada 10 ms cuando cualquiera de las tiras usa estrobo, conservando 50 ms para el resto y evitando transmitir durante respuesta RDM. Pruebas de todos los modos y del refresco integrado para confirmar que se ven fases encendidas/apagadas a 20 Hz.

Estrobo acelerado: suite completa y compilación superadas; paquete 32 560 bytes, CRC FTC 2E23. Carga RDM pendiente por `Resource busy` en la interfaz. Cerrar QLC+ u otra aplicación que la ocupe y repetir el lanzador RDM. Registro `build/controlador/carga-estrobo-20261028.log`.

QLC+ maestro: el workspace guardado había pasado a Generic/Generic/19 Channel, y la definición instalada seguía en v0.0.7. Se restaura la referencia Salto Temporal/ST-01/v0.0.9 sin modificar el diseño de consola guardado por el usuario y se actualiza la definición instalada. Canales 1–4, 8 y 19 son Effect/LTP; los 13 canales RGB/pilotos son Intensity. Reabrir workspace con la definición actualizada para aplicar en QLC+. Copia previa del workspace conservada en `build/qlcplus/`.

Tras liberar QLC+, la carga RDM del estrobo acelerado finalizó correctamente: validación/commit/reinicio completados, versión 0.0.9 confirmada. El controlador ya incorpora estrobo de 8–20 Hz y refresco acelerado durante ese efecto, además de KITT con cabeza al 100 % y cola 10–1 %. Registro `build/controlador/carga-estrobo-20261028.log`.

El usuario observó estrobo irregular a 8–20 Hz y pidió restaurarlo. Se recuperan los periodos originales de 250–100 ms (4–10 Hz) en todos los modos de Barra/Tira y el refresco uniforme de 50 ms; se retira el refresco especial de 10 ms. KITT y la corrección de canales de QLC+ no cambian.

Restauración del estrobo completada: suite completa superada y carga RDM validada con commit/reinicio y versión 0.0.9 confirmada. Estado actual del controlador: estrobo 4–10 Hz, refresco 50 ms, KITT con cabeza 100 % y cola 10–1 %. Registro `build/controlador/restaurar-estrobo-20261028.log`.
