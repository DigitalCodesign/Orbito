#include <Orbito.h>

// Callback para el interruptor
void onSwitch(bool state) {
    if (state) {
        Orbito.Display.fillScreen(0xFFFF); // Blanco
        Orbito.Action.say("LUZ ON");
    } else {
        Orbito.Display.fillScreen(0x0000); // Negro
        Orbito.Action.say("LUZ OFF");
    }
}

void setup() {
    Orbito.begin();

    // Iniciar BLE
    Orbito.Remote.initDashboard("Orbito_Robot");

    // Agregar controles
    Orbito.Remote.addSwitch("Luz Pantalla", onSwitch);

    Orbito.Display.consoleLog("Abre la App y conecta");
}

void loop() {
    Orbito.update(); // Mantiene la conexión BLE viva
}
