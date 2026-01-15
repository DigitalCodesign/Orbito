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
  static int last_calidad = -1; 
  static String last_ip = "";

  if (millis() - last_check > 500) {

    last_check = millis();

    // ------------------------------------------------------
    // 1. GESTIÓN DE IP (Solo pintar si cambia)
    // ------------------------------------------------------
    String current_ip = Orbito.Connect.getIP();
    
    if (current_ip != last_ip) {

      // A. Borrar lo anterior (Caja negra en la zona de la IP)
      Orbito.Display.fillRect(10, 10, 300, 20, 0x0000); 

      // B. Escribir lo nuevo
      Orbito.Display.setCursor(10, 10);
      Orbito.Display.print("IP: " + current_ip);

      // C. Actualizar memoria
      last_ip = current_ip;

    }

    // ------------------------------------------------------
    // 2. GESTIÓN DE SEÑAL (Solo pintar si cambia)
    // ------------------------------------------------------
    int calidad = Orbito.Connect.getSignalQuality();

    // Solo redibujamos si el valor ha cambiado para evitar parpadeos
    if (calidad != last_calidad) {

      // --- DIBUJAR BARRA GRÁFICA ---
      // A. Borrar barra anterior (Rectángulo negro completo)
      Orbito.Display.fillRect(10, 50, 200, 30, 0x0000);

      // B. Calcular color
      uint16_t color = (calidad > 50) ? 0x07E0 : 0xF800; 

      // C. Dibujar barra nueva
      Orbito.Display.fillRect(10, 50, calidad * 2, 30, color);

      // --- DIBUJAR TEXTO PORCENTAJE ---
      // A. Borrar texto anterior
      Orbito.Display.fillRect(10, 90, 200, 20, 0x0000);

      // B. Escribir texto nuevo
      Orbito.Display.setCursor(10, 90);
      Orbito.Display.print("Senal: " + String(calidad) + "%");

      // C. Actualizar memoria
      last_calidad = calidad;

    }

  }

}

