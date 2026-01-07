#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.fillScreen(0xFFFF); // Fondo Blanco

    Orbito.Display.setTextColor(0x0000); // Texto Negro

    Orbito.Display.setFont(1);
    Orbito.Display.setCursor(10, 20);
    Orbito.Display.print("Texto Pequeno (Size 1)");

    Orbito.Display.setFont(2);
    Orbito.Display.setCursor(10, 60);
    Orbito.Display.setTextColor(0xF800); // Rojo
    Orbito.Display.print("Texto Mediano");

    Orbito.Display.setFont(3);
    Orbito.Display.setCursor(10, 120);
    Orbito.Display.setTextColor(0x001F); // Azul
    Orbito.Display.print("GRANDE!");
}

void loop() {
    Orbito.update();
}
