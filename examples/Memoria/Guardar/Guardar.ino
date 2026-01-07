#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Test Memoria Flash");

    // Escribir
    String data = "Inicio: " + String(millis()) + "ms";
    if (Orbito.Storage.writeFile("/log_inicio.txt", data)) {
        Orbito.Display.consoleLog("Archivo escrito OK");
    } else {
        Orbito.Display.consoleLog("Error escritura");
    }

    // Leer
    if (Orbito.Storage.exists("/log_inicio.txt")) {
        String leido = Orbito.Storage.readFile("/log_inicio.txt");
        Orbito.Display.consoleLog("Contenido:");
        Orbito.Display.consoleLog(leido);
    }
}

void loop() {
    Orbito.update();
}