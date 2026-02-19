#include <Orbito.h>

void setup() {
    // Iniciar Serial (USB Nativo)
    Serial.begin(115200);

    // Inicialización obligatoria
    Orbito.begin();
    Orbito.Display.consoleLog("Prueba de Sistema");
    Orbito.Display.consoleLog("Pulsa el boton...");
}

void loop() {
    Orbito.update(); // Mantenimiento interno

    // Detectar pulsación del botón integrado
    if (Orbito.System.getButtonStatus()) {
        Orbito.Display.consoleLog("Boton pulsado!");

        // Tono ascendente
        Orbito.System.tone(1000, 100); // 1000Hz, 100ms
        delay(150);
        Orbito.System.tone(2000, 100); // 2000Hz, 100ms

        // Esperar a que se suelte para no repetir
        while(Orbito.System.getButtonStatus()) delay(10);
    }
}
