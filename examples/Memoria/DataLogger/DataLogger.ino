#include <Orbito.h>

String filename = "/datos.csv";

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Iniciando Datalogger...");

    // 1. Crear cabeceras si el archivo no existe
    if (!Orbito.Storage.exists(filename)) {
        Orbito.Storage.writeFile(filename, "Tiempo_ms,Valor_Sensor\n");
        Orbito.Display.consoleLog("Archivo creado.");
    } else {
        Orbito.Display.consoleLog("Archivo existente. Anadiendo...");
    }
}

void loop() {
    Orbito.update();

    static unsigned long last_log = 0;
    // Guardar datos cada 5 segundos
    if (millis() - last_log > 5000) {
        last_log = millis();

        // Simular un valor de sensor (ej. temperatura)
        int sensorSimulado = random(20, 35);

        // Crear línea CSV: "15000,24"
        String linea = String(millis()) + "," + String(sensorSimulado) + "\n";

        // Guardar (Append)
        if (Orbito.Storage.appendFile(filename, linea)) {
            Orbito.Display.consoleLog("Guardado: " + String(sensorSimulado));
        } else {
            Orbito.Display.consoleLog("Error al guardar!");
        }
    }
}
