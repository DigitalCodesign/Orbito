#include <Orbito.h>

int contador_fotos = 0;

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Modo Timelapse");

    // Configurar calidad media para ahorrar espacio
    Orbito.Vision.setResolution(FRAMESIZE_VGA); // 640x480
    Orbito.Vision.setQuality(15);
}

void loop() {
    Orbito.update();

    static unsigned long t = 0;
    // Cada 10 segundos
    if (millis() - t > 10000) {
        t = millis();

        // Generar nombre archivo: "/lapso_0.jpg", "/lapso_1.jpg"...
        String nombre = "/lapso_" + String(contador_fotos) + ".jpg";

        Orbito.Display.consoleLog("Capturando: " + nombre);

        // Guardar
        bool exito = Orbito.Vision.saveSnapshot(nombre);

        if (exito) {
            contador_fotos++;
            Orbito.Display.consoleLog("Guardada OK");
        } else {
            Orbito.Display.consoleLog("Error Flash Llena?");
        }
    }
}
