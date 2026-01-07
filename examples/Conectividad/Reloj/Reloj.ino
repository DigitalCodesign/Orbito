#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Connect.connect("TU_WIFI", "TU_CLAVE");

    Orbito.Display.consoleLog("Esperando Hora NTP...");
}

void loop() {
    Orbito.update();

    static unsigned long t = 0;
    if (millis() - t > 1000) {
        t = millis();
        // Imprimir hora cada segundo
        String hora = Orbito.Connect.getTime();

        // Si la hora es válida (no es 1970)
        if (hora.length() > 5) {
            Orbito.Display.fillScreen(0x0000); // Limpiar
            Orbito.Display.setCursor(50, 100);
            Orbito.Display.setFont(3);
            Orbito.Display.print(hora);
        }
    }
}
