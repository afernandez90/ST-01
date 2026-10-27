# Referencias de los visualizadores

Consulta: 27 de octubre de 2026. El usuario selecciona **CL5611AH** para las pruebas pequeñas, sustituyendo la elección inicial 5101AS. Para el montaje grande mantiene **50101AS**, rojo, de 5 pulgadas y cátodo común.

Las referencias encontradas son **CL5611AH de XLITX** y **ELS-50101AS de Etnal/YTNLED**. El fabricante de los visualizadores grandes adquiridos no está confirmado: compartir el código 50101AS no demuestra equivalencia eléctrica ni de patillaje. Comprobar las unidades reales antes de montar o intercambiar.

## Valores publicados

| Parámetro | CL5611AH, referencia XLITX | ELS-50101AS, referencia Etnal |
|---|---|---|
| Tamaño de dígito | 0,56 pulgadas | 5 pulgadas |
| Polaridad | Cátodo común | Cátodo común |
| Tensión directa de segmento rojo | 1,8 V típica a 10 mA | 12,6–14,0 V; tabla con IF de referencia 20 mA |
| Tensión directa del punto decimal | Sin valor separado en la tabla | 5,4–6,0 V |
| Corriente | 30 mA por chip LED como máximo absoluto; no es corriente de diseño | IF de referencia 20 mA; no presentarlo como máximo absoluto |
| Pulsos | No utilizar sus límites pulsados para el funcionamiento estático | No utilizar sus límites pulsados para el funcionamiento estático |

Fuentes primarias:

- [Copia local: CL5611AH de XLITX](datasheets/CL5611AH-XLITX.pdf) · [origen](https://www.xlitx.com/datasheet/CL5611AH.pdf), páginas 1–3; diagrama de la página 2 inspeccionado visualmente.
- [Copia local: ELS-50101A/B de Etnal](datasheets/ELS-50101A-B-Etnal.pdf) · [origen](https://www.ytnled.com/download/1/ELS-50101A-B.pdf), tabla y diagrama de la única página inspeccionados visualmente.
- [Página de producto ELS-50101A/B de Etnal](https://www.ytnled.com/product/1/ELS-50101A-B.html).

## Patillaje de referencia

Numeración según los dibujos de cada fabricante, mirando el frontal con el punto decimal abajo a la derecha. Confirmar la marca de pin 1 y la orientación física de la unidad.

| Función | CL5611AH | ELS-50101AS |
|---|---:|---:|
| A | 7 | 7 |
| B | 6 | 6 |
| C | 4 | 4 |
| D | 2 | 3 |
| E | 1 | 2 |
| F | 9 | 9 |
| G | 10 | 10 |
| DP | 5 | 5 |
| Cátodos comunes | 3 y 8 | 1 y 8 |

**No son intercambiables pin por pin:** cambian los pines 1, 2 y 3. Mantener el cableado lógico A–G/DP del controlador y usar un adaptador o arnés específico para cada visualizador; no hace falta cambiar los códigos DMX por este cambio de patillaje.

## Alimentación y resistencias

Se conserva la etapa propuesta **74HC595 + TBD62783APG** también para el pequeño. La lógica del Mega y los 74HC595 sigue a 5 V. La alimentación de la etapa de potencia se adapta al visualizador, con masa común y sin aplicar la tensión elevada a la lógica.

La tensión directa es la caída en los LED a la corriente indicada, no una tensión de alimentación que pueda aplicarse sin resistencia. Cada segmento y el punto decimal requieren su propia resistencia:

`R = (Vfuente − Vcaída_driver − VF) / I`

Para el pequeño se evaluará una fuente de 5 V con corriente conservadora. Si el grande se comporta como la referencia Etnal, 5 V no bastan y una fuente de 12 V tampoco ofrece margen para alcanzar el punto de referencia de 20 mA. Hace falta una tensión superior a VF más la caída del driver y el margen de la resistencia; fijarla mediante prueba con corriente limitada. El punto decimal requiere una resistencia distinta por su menor VF. Comprobar también disipación de resistencias y driver, y tensión nominal de los condensadores de la alimentación de potencia.

Aunque el control digital se conserve, el cambio del pequeño al grande puede requerir **adaptador de pines, tensión de potencia y resistencias**. Las cifras publicadas orientan las pruebas; no sustituyen las medidas de los componentes recibidos.

## Interacción con Arduino y etapas de potencia

El Mega convierte cada valor DMX en ocho bits para A–G y DP. Los pines 51 (datos) y 52 (reloj) envían 32 bits a cuatro 74HC595 encadenados; el pin 49 aplica el patrón completo mediante el latch. El pin 48 se reserva para habilitar o apagar las salidas. Cada 74HC595 controla las ocho entradas de un TBD62783APG; este suministra corriente desde la fuente de potencia a los ánodos. Los cátodos comunes vuelven a masa. El circuito sigue pendiente de montaje y validación; no reutilizar el esquema de conexiones físicas de los antiguos TPIC6B595.

La lógica permanece a 5 V. La fuente de potencia del driver puede ser distinta según el visualizador, con masa común. La corriente de segmentos no debe atravesar las salidas del Mega ni las del 74HC595. La estructura es estática, sin multiplexación: cada dígito tiene su propio registro, driver y ocho resistencias.

## Resistencias y condensadores de la propuesta

| Componente | Cantidad orientativa | Función y colocación |
|---|---|---|
| Resistencias de segmentos | Ocho por dígito, 32 para cuatro dígitos | Una en serie con cada A–G y DP; limitar corriente. No usar una sola resistencia en el cátodo común. |
| Pull-up de habilitación | Uno para la línea compartida, 10 kΩ como punto de partida | Entre /OE de los 74HC595 y 5 V: mantener las salidas deshabilitadas mientras el Mega se reinicia. |
| Pull-downs de entradas de potencia | Ocho por driver, 10 kΩ como punto de partida | Entre cada entrada del TBD62783APG y masa: definir apagado cuando las salidas del 74HC595 están en alta impedancia. |
| Cerámicos de desacoplo | 100 nF junto a cada IC, ocho para cuatro registros y cuatro drivers | Entre alimentación y GND, con conexiones cortas; uno en el rail lógico de cada 74HC595 y otro en el rail de potencia de cada driver. |
| Condensador de reserva | 10–100 µF iniciales por entrada de alimentación del módulo | Entre alimentación y masa para amortiguar cambios de carga; cantidad y capacidad finales según cableado y medidas. |

Al arrancar, deshabilitar las salidas, desplazar ceros y aplicar el latch antes de habilitar. /SRCLR debe permanecer en un nivel definido; borrar el registro de desplazamiento no sustituye a aplicar el latch del patrón apagado.

Ejemplo únicamente para el pequeño: con fuente de 5 V y VF de 1,8 V, despreciando la caída del driver, 330 Ω limita a unos 9,7 mA y disipa unos 0,031 W. Una resistencia de 0,25 W ofrece margen en ese ejemplo. Verificar el punto decimal por separado y medir la corriente real. No trasladar ese valor a los grandes.

La potencia de cada resistencia se calcula como `P = I² × R`; elegir potencia nominal con margen. Los condensadores no limitan la corriente LED ni sustituyen las resistencias. Su tensión nominal debe superar el rail donde se conectan; los de la etapa de potencia deben soportar su tensión mayor. Respetar la polaridad de los electrolíticos. Los valores de reserva y de polarización son puntos de partida, no un circuito ya validado.

Documentación de los circuitos: [74HC595, TI](https://www.ti.com/lit/ds/symlink/sn74hc595.pdf) y [TBD62783A, Toshiba](https://toshiba-semicon-storage.com/info/docget.jsp?did=30523&prodName=TBD62783AFNG). La hoja de TI recomienda desacoplo de 100 nF próximo a la alimentación. Comprobar los límites simultáneos de corriente y disipación del driver, además de los de cada canal.

## Propuesta de elevador desde la fuente de 12 V

El usuario dispone de fuentes de 12 V y 5 V y alimenta el Mega por el conector de barril. Mantener la rama del barril y la de pilotos a 12 V. Un elevador DC–DC independiente puede alimentar solo los drivers de los visualizadores grandes; no elevar la tensión del Mega ni de los 74HC595. Compartir masa.

Se recomienda como candidato **Pololu U3V70A, referencia 2890**, ajustable de 4,5 a 20 V, configurado inicialmente como propuesta de diseño a **18 V**. No consta que se haya comprado ni probado. [Especificaciones del fabricante](https://www.pololu.com/product/2890). Un único módulo puede alimentar los cuatro drivers, sujeto a comprobar temperaturas y funcionamiento con las cargas reales. Ajustar y medir su salida antes de conectar los visualizadores.

Presupuesto calculado para cuatro dígitos estáticos, ocho segmentos por dígito y una corriente de diseño de hasta 20 mA por segmento, no un máximo absoluto atribuido al display:

- Corriente de salida: `4 × 8 × 0,020 A = 0,64 A`.
- Potencia a 18 V: `18 × 0,64 = 11,52 W`.
- Corriente de entrada estimada a 12 V y rendimiento supuesto del 85 %: `11,52 / (12 × 0,85) = 1,13 A`.
- Dimensionar como mínimo para 18 V y 1 A continuo en estas condiciones, con margen térmico. Reservar aproximadamente 2 A de capacidad en la fuente de 12 V para esta rama, además del consumo de los demás elementos. La cifra de corriente de interruptor o entrada anunciada por un elevador no equivale a su corriente continua de salida.

Para la primera prueba conservadora a 18 V: siete resistencias de **560 Ω / 1 W** y una de **1,5 kΩ / 1 W** para DP por dígito. Con los VF de referencia y sin descontar la caída del driver, los siete segmentos quedarían aproximadamente entre 7,1 y 9,6 mA y DP entre 8,0 y 8,4 mA. La disipación calculada máxima con esos valores de referencia es aproximadamente 0,052 W por resistencia de segmento y 0,106 W para DP. Repetir medidas con las unidades reales; no tomar esos intervalos como límites garantizados a otra corriente o temperatura.

En el conjunto de cuatro dígitos, esa prueba consumiría aproximadamente 0,23–0,31 A de la salida de 18 V, unos 4,2–5,6 W. El presupuesto superior de 0,64 A deja margen para estudiar más brillo posteriormente; las resistencias iniciales no producen 20 mA. Los condensadores propuestos de 50 V sirven en el rail de 18 V; las resistencias de polarización y desacoplos permanecen como se describe arriba.
