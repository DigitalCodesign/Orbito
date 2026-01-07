#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Me voy a dormir...");
    Orbito.Display.consoleLog("Pulsame para despertar");
    delay(2000);

    // Hibernar indefinidamente.
    // Param 1 (-1): Usar botón por defecto.
    // Param 2 (0): Despertar cuando la señal sea LOW (0).
    Orbito.System.hibernate(-1, 0); 
}

void loop() {
    // Este código nunca se ejecuta porque el robot se duerme en el setup
    // y al despertar reinicia desde el principio.
}
