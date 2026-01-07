#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Gestor de Archivos");

    // Crear un archivo basura para probar
    Orbito.Storage.writeFile("/basura.txt", "Contenido inutil");

    delay(1000);

    if (Orbito.Storage.exists("/basura.txt")) {
        Orbito.Display.consoleLog("Borrando archivo...");

        // BORRAR
        if (Orbito.Storage.remove("/basura.txt")) {
            Orbito.Display.consoleLog("Eliminado con exito.");
        } else {
            Orbito.Display.consoleLog("Fallo al eliminar.");
        }
    } else {
        Orbito.Display.consoleLog("El archivo no existe.");
    }

    // Mostrar espacio libre final
    int libre = Orbito.Storage.getTotalSpace() - Orbito.Storage.getUsedSpace();
    Orbito.Display.consoleLog("Libre: " + String(libre) + " bytes");
}

void loop() {
    Orbito.update();
}