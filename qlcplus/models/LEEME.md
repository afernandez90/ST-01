# Maqueta 3D provisional de ST-01

Modelo estático para visualizar la disposición de los componentes en QLC+ 5. No es un plano eléctrico ni una representación de dimensiones definitivas.

- Envolvente: cubo de 1.200 × 1.200 × 1.200 mm, elevado 80 mm sobre cuatro patas.
- Cara A: frontal; cuatro dígitos de siete segmentos con muestra **1985**, Barra LED RGB y cuatro pilotos (azul, verde, naranja, rojo).
- Cara B: lateral derecho, perpendicular a Cara A y compartiendo una sola arista; piloto naranja B (LNB), dos blancos, Tira LED COB y piloto azul de estado.
- Los colores, el recorrido rectangular de la Tira y todas las medidas son aproximaciones visuales.
- Los números y luces del modelo son estáticos: **no siguen los canales DMX**. La personalidad y los controles del dispositivo no cambian.

## Verlo en QLC+

Reabrir `../SaltoTemporal.qxw`, entrar en el contexto de equipos y funciones y seleccionar la **vista 3D**. La maqueta ya está añadida como objeto personalizado en el proyecto, con una rotación de −30° para mostrar ambas caras. Ajustar el zoom y orbitar si es necesario.

Si se importa por separado desde Objetos personalizados → ＋, seleccionar `ST-01-cubo.obj` y conservar `ST-01-cubo.mtl` en la misma carpeta. El OBJ usa metros; la posición del objeto en el espacio de trabajo usa milímetros. El modelo está colocado en el origen, sobre el suelo.

`ST-01-cubo-preview.png` es una vista previa renderizada por software a partir de la misma geometría, no una captura de QLC+.

## Regenerar

```sh
/usr/bin/python3 qlcplus/models/generar_cubo.py
```

El generador usa únicamente la biblioteca estándar de Python y produce OBJ y MTL. La importación se ha comprobado con el lector Assimp incluido en QLC+ 5.3.0: 729 mallas y 14 materiales.
