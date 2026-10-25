# Firmware Arduino de ST-01

Esta carpeta contendrá el firmware para Arduino Mega.

El firmware debe implementar la personalidad DMX de 19 canales `Control v0.0.6`, documentada en [`../../docs/MAPA_DE_CANALES_DMX.md`](../../docs/MAPA_DE_CANALES_DMX.md). Recibirá DMX por RX1 (pin 19, USART1) mediante una interfaz RS-485 aislada a 250 kbaud, 8N2.

No cambie los desplazamientos de canal sin publicar un nuevo modo en la definición del fixture y actualizar la documentación de usuario.

La implementación actual del prototipo está en `../pruebas/recepcion_dmx`. Incluye las salidas definidas en el [cableado propuesto](../../docs/CABLEADO_PROPUESTO.md), identificación conjunta y el enlace al [cargador FTC](../../docs/ACTUALIZACION_RDM.md). Esta carpeta se mantiene reservada para la versión final tras validar el montaje.
