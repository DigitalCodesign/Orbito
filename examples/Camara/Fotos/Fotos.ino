#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Pulsa boton para FOTO");
}

void loop() {
    Orbito.update();

    if (Orbito.System.getButtonStatus()) {
        Orbito.Display.fillScreen(0x0000);

        // 1. Capturar
        camera_fb_t* frame = Orbito.Vision.snapshot();

        if (frame) {
        // 2. Dibujar
        Orbito.Display.drawSnapshot(frame);
        // 3. Liberar
        Orbito.Vision.release(frame);
        }

        delay(2000); // Ver foto 2 segundos
        Orbito.Display.consoleLog("Listo para otra.");
    }
}
