#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Acerca una tarjeta NFC");
}

void loop() {
    Orbito.update();

    if (Orbito.Remote.isFieldPresent()) {

        // Leer ID único
        String uid = Orbito.Remote.readTagUID();
        Orbito.Display.consoleLog("UID: " + uid);

        // Leer Texto (si tiene formato NDEF)
        String texto = Orbito.Remote.readTagText();
        if (texto != "") {
        Orbito.Display.consoleLog("Dice: " + texto);
        }

        delay(2000); // Evitar lecturas múltiples seguidas
    }
}