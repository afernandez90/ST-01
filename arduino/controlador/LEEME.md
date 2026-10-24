# Firmware Arduino de ST-01

Esta carpeta contendrá el firmware para Arduino Mega.

El firmware debe implementar la personalidad DMX de 20 canales `Control v0.0.1`, documentada en [`../../docs/MAPA_DE_CANALES_DMX.md`](../../docs/MAPA_DE_CANALES_DMX.md). Recibirá DMX por `Serial1` mediante una interfaz RS-485 aislada a 250 kbaud, 8N2.

No cambie los desplazamientos de canal sin publicar un nuevo modo en la definición del fixture y actualizar la documentación de usuario.
