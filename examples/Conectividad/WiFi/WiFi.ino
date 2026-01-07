#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Display.consoleLog("Conectando a WiFi...");

    // Conectar a la red
    Orbito.Connect.connect("TU_WIFI", "TU_CLAVE");

    // Limpiar pantalla para dibujar la interfaz
    Orbito.Display.fillScreen(0x0000);
}

void loop() {
    Orbito.update();

    static unsigned long last_check = 0;
    if (millis() - last_check > 500) {
        last_check = millis();

        // Obtener calidad (0-100%)
        int calidad = Orbito.Connect.getSignalQuality();
        String ip = Orbito.Connect.getIP();

        // 1. Dibujar IP
        Orbito.Display.setTextColor(0xFFFF);
        Orbito.Display.setCursor(10, 10);
        Orbito.Display.print("IP: " + ip);

        // 2. Dibujar Barra de Señal
        // Borrar barra anterior (dibujando rectángulo negro)
        Orbito.Display.fillRect(10, 50, 200, 30, 0x0000);

        // Calcular color según fuerza (Verde bueno, Rojo malo)
        uint16_t color = (calidad > 50) ? 0x07E0 : 0xF800; // Verde o Rojo

        // Dibujar barra nueva (ancho proporcional a la calidad)
        Orbito.Display.fillRect(10, 50, calidad * 2, 30, color);

        // Texto porcentaje
        Orbito.Display.setCursor(10, 90);
        Orbito.Display.print("Senal: " + String(calidad) + "%");
    }
}