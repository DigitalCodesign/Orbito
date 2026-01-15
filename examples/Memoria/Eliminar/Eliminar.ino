#include <Orbito.h>

void setup() {

  Orbito.begin();
  Orbito.Display.consoleLog("--- MODO ELIMINAR ---");

  // 1. Comprobar si hay algo guardado (del ejemplo anterior)
  if (Orbito.Storage.exists("/log.txt")) {

    // Mostrar lo que hay antes de borrarlo
    String contenido = Orbito.Storage.readFile("/log.txt");
    Orbito.Display.consoleLog("Encontrado:");
    Orbito.Display.consoleLog(contenido);

    delay(1000); 

    // 2. Borrar
    Orbito.Display.consoleLog("Borrando...");

    if (Orbito.Storage.remove("/log.txt")) {
      Orbito.Display.setTextColor(0xF800); // Rojo
      Orbito.Display.consoleLog("DATOS BORRADOS.");
    } else {
      Orbito.Display.consoleLog("Fallo al borrar.");
    }

  } else {

    // Si no hay nada escrito
    Orbito.Display.consoleLog("Memoria vacia.");
    Orbito.Display.consoleLog("Usa Guardar.ino primero");

  }

}

void loop() {

  Orbito.update();

}
