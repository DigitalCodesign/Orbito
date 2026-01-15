#include <Orbito.h>

// Definimos el nombre del archivo aquí para que sea fácil de cambiar
String nombreArchivo = "/test_orbito.txt";

void setup() {

  Orbito.begin();
  Orbito.Display.consoleLog("--- MODO GUARDAR ---");

  // NOTA: Si es la primera vez absoluta que usas la memoria y falla,
  // descomenta la siguiente línea una sola vez para formatear:
  // Orbito.Storage.format(); 

  // 1. Preparar datos
  String mensaje = "Dato guardado a " + String(millis()) + "ms";

  // 2. Escribir (Sobrescribe si ya existe)
  if (Orbito.Storage.writeFile(nombreArchivo, mensaje)) {

    Orbito.Display.consoleLog("Archivo CREADO:");
    Orbito.Display.consoleLog(nombreArchivo);

  } else {

    Orbito.Display.consoleLog("ERROR de escritura");
    Orbito.Display.consoleLog("¿Memoria llena o sin formato?");

  }

  // 3. Verificación inmediata (Leer lo que acabamos de guardar)
  if (Orbito.Storage.exists(nombreArchivo)) {

    String lectura = Orbito.Storage.readFile(nombreArchivo);
    Orbito.Display.consoleLog("Contenido real:");
    Orbito.Display.consoleLog(lectura);

  }

}

void loop() {

  Orbito.update();

}
