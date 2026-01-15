#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("INICIADO");
    delay(1000);
}

void loop() {
    // 1. Fase Activa
    Orbito.Display.fillScreen(0x0000); // Limpiar
    Orbito.Display.setCursor(0,0);
    Orbito.Display.consoleLog("TRABAJANDO...");
    delay(2000);

    // 2. Fase Dormida
    Orbito.Display.consoleLog("Durmiendo...");
    delay(500); 

    // Se duerme hasta que pulses el botón (PIN 48)
    // El -1 indica que no usamos disparador externo extra, solo el botón base
    Orbito.System.hibernate(-1, 0); 

    // 3. Al despertar continúa inmediatamente aquí
    Orbito.Display.consoleLog("¡DESPIERTO!");
    delay(1000);
}
