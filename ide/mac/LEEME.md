# Configuración de VS Code para Mac

Esta carpeta contiene la configuración compartida de VS Code y el archivo `ST-01.code-workspace` para esta plataforma. La configuración activa en la raíz del repositorio permanece local e ignorada por Git.

## Uso

1. Abra `ide/mac/ST-01.code-workspace` en VS Code. Su carpeta de trabajo apunta a la raíz del repositorio mediante `../..`; mantenga este archivo en su ubicación versionada.
2. Copie el contenido de `ide/mac/.vscode/` a `.vscode/` en la raíz, incluida la configuración y los tres guiones auxiliares. Si ya tiene una configuración local, combine los archivos que necesite.
3. Instale la extensión C/C++ recomendada y las dependencias Arduino indicadas en `arduino/pruebas/LEEME.md`.
4. Revise las rutas de herramientas y bibliotecas antes de compilar.

Las tareas y opciones de Ejecutar y depurar permiten compilar, cargar y monitorizar los sketches, además de ejecutar las pruebas RDM. Los guiones están pensados para ejecutarse desde la copia activa en `.vscode/`; sus rutas relativas apuntan a la raíz del repositorio.

## Rutas y dependencias

- Arduino CLI: `/opt/homebrew/bin/arduino-cli`.
- Python: `/usr/bin/python3`.
- IntelliSense: las rutas usan `${userHome}` para el directorio del usuario. Se espera Arduino AVR Boards 1.8.8, AVR GCC 7.3.0-atmel3.6.1-arduino7 y las bibliotecas en `Documents/Arduino/libraries`; adapte las rutas si su instalación es distinta.
- Las pruebas sin placa requieren el compilador C++ del sistema con sanitizadores.

Los puertos del Mega se detectan mediante Arduino CLI. Si hay varios Mega conectados, seleccione el puerto mediante `ST01_PORT` o desconecte los demás. Las pruebas físicas RDM necesitan una interfaz compatible con el transporte serie de la herramienta; cierre las aplicaciones que ocupen sus puertos.

Las modificaciones en la copia activa no actualizan automáticamente estas plantillas. Para compartirlas, copie aquí los cambios pertinentes sin añadir rutas personales ni nombres de puerto fijos. Otras plataformas pueden incorporar sus configuraciones bajo carpetas hermanas de `ide/mac/`.
