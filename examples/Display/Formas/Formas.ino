#include <Orbito.h>

void setup() {
    Orbito.begin();

    // Limpiar pantalla en negro
    Orbito.Display.fillScreen(0x0000);

    // Dibujar cara simple con primitivas
    Orbito.Display.fillCircle(160, 120, 100, 0xFFE0); // Cara Amarilla
    Orbito.Display.fillCircle(130, 100, 15, 0x0000);  // Ojo Izq Negro
    Orbito.Display.fillCircle(190, 100, 15, 0x0000);  // Ojo Der Negro
    Orbito.Display.fillRect(110, 160, 100, 10, 0xF800); // Boca Roja (Rectángulo)
}

void loop() {
    Orbito.update();
}
