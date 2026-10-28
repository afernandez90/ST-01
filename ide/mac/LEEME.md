# Configuración de VS Code para Mac

Esta carpeta contiene la configuración compartida de VS Code y el archivo `ST-01.code-workspace` para esta plataforma. La configuración activa en la raíz del repositorio permanece local e ignorada por Git.

## Uso

1. Abra `ide/mac/ST-01.code-workspace` en VS Code. Su carpeta de trabajo apunta a la raíz del repositorio mediante `../..`; mantenga este archivo en su ubicación versionada.
2. Copie el contenido de `ide/mac/.vscode/` a `.vscode/` en la raíz, incluida la configuración y los tres guiones auxiliares. Si ya tiene una configuración local, combine los archivos que necesite.
3. Instale la extensión C/C++ recomendada y las dependencias Arduino indicadas en `arduino/pruebas/LEEME.md`.
4. Revise las rutas de herramientas y bibliotecas antes de compilar.

La tarea **RDM: compilar y cargar controlador** compila el firmware, genera el paquete ST01, lo transmite por RDM y consulta la versión después del reinicio. También F5 y la carga del sketch activo utilizan RDM cuando el sketch es `controlador`; las pruebas independientes conservan su carga USB. El monitor USB se abre de forma independiente y no es necesario para actualizar por RDM. Los guiones están pensados para ejecutarse desde la copia activa en `.vscode/`; sus rutas relativas apuntan a la raíz del repositorio.

## Rutas y dependencias

- Arduino CLI: `/opt/homebrew/bin/arduino-cli`.
- Python: `/usr/bin/python3`.
- IntelliSense: las rutas usan `${userHome}` para el directorio del usuario. Se espera Arduino AVR Boards 1.8.8, AVR GCC 7.3.0-atmel3.6.1-arduino7 y las bibliotecas en `Documents/Arduino/libraries`; adapte las rutas si su instalación es distinta.
- IntelliSense incluye explícitamente las cabeceras de AVR y las bibliotecas Wire, EEPROM y SPI, además de la definición del ATmega2560. Si persiste un aviso de cabeceras tras actualizar la copia activa, ejecute `C/C++: Reset IntelliSense Database` y después `Developer: Reload Window` desde la paleta de comandos.
- Las pruebas sin placa requieren el compilador C++ del sistema con sanitizadores.

Los puertos del Mega se detectan mediante Arduino CLI. Si hay varios Mega conectados, seleccione el puerto mediante `ST01_PORT` o desconecte los demás. Las pruebas físicas RDM necesitan una interfaz compatible con el transporte serie de la herramienta; cierre las aplicaciones que ocupen sus puertos.

Las modificaciones en la copia activa no actualizan automáticamente estas plantillas. Para compartirlas, copie aquí los cambios pertinentes sin añadir rutas personales ni nombres de puerto fijos. Otras plataformas pueden incorporar sus configuraciones bajo carpetas hermanas de `ide/mac/`.

## Actualizaciones del controlador por RDM

La ruta habitual de actualización es RDM. Cierre QLC+ u otra aplicación que ocupe la interfaz y ejecute `Tasks: Run Task` → **RDM: compilar y cargar controlador**. No hace falta conectar el USB del Mega. El lanzador común está en `herramientas/cargar_rdm.sh`; las tareas fijan las rutas de Arduino CLI y Python de esta plataforma.

Con una única interfaz compatible se detecta el puerto automáticamente. Si necesita elegirlo, configure `ST01_RDM_PORT` en el entorno de VS Code o en `options.env` de la tarea local. `ST01_RDM_UID` permite elegir otro UID; por defecto se utiliza el prototipo `7ff0:66601337`. No versionar nombres de puerto personales. La copia activa y la plantilla compartida deben mantener la misma ruta RDM.

Como respaldo explícito existe la tarea **USB: cargar controlador (respaldo)**: compila, carga por el USB del Mega y abre el monitor serie. Requiere conectar el Mega por USB y cerrar cualquier monitor que ocupe su puerto. La ruta predeterminada y F5 continúan utilizando RDM.
