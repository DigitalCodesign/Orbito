#include <Orbito.h>

const char* SSID = "TU_WIFI";
const char* PASS = "TU_CLAVE";

void setup() {
    Orbito.begin();

    // 1. Conectar WiFi
    Orbito.Display.consoleLog("Conectando WiFi...");
    Orbito.Connect.connect(SSID, PASS);

    // 2. Iniciar Streaming
    Orbito.Vision.setResolution(FRAMESIZE_QVGA); // 320x240 (Fluido)
    Orbito.Vision.startWebStream(); // Inicia en puerto 81

    Orbito.Display.consoleLog("Video Listo:");
    Orbito.Display.consoleLog("http://" + Orbito.Connect.getIP() + ":81");
}

void loop() {
    Orbito.update();
}