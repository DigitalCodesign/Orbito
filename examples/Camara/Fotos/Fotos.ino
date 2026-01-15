#include <Orbito.h>

void setup() {

    // 1. INICIAR EL MONITOR Y EL ROBOT
    Serial.begin(115200);
    Orbito.begin(); 

    Orbito.Display.consoleLog("Iniciando...");

    // 2. CONFIGURAR LA CÁMARA
    // Igual que en tu ejemplo de WiFi pones MODE_STREAMING,
    // aquí ponemos MODE_AI para obtener formato RGB565 (necesario para display)
    Orbito.Vision.setMode(CameraHandler::MODE_AI);

    // 3. MENSAJE DE LISTO
    Orbito.Display.consoleLog("CAMARA LISTA");
    Orbito.Display.consoleLog("Pulsa boton...");

}

void loop() {

    // Mantenimiento del sistema
    Orbito.update();

    // Detección del botón
    if (Orbito.System.getButtonStatus()) {

        Orbito.Display.consoleLog("Capturando...");

        // 1. Limpiar pantalla para pintar la foto limpia
        Orbito.Display.fillScreen(0x0000);

        // 2. Captura Real
        camera_fb_t* frame = Orbito.Vision.snapshot();

        if (frame) {

            // 3. Dibujar
            Orbito.Display.drawSnapshot(frame);
            
            // 4. Liberar Memoria (OBLIGATORIO)
            Orbito.Vision.release(frame);

        } else {

            Orbito.Display.consoleLog("Error captura");

        }

        delay(2000); // Mantener la foto 2 segundos en pantalla
        
        // Restaurar interfaz de texto
        Orbito.Display.fillScreen(0x0000);
        Orbito.Display.setCursor(0,0);
        Orbito.Display.consoleLog("Listo para otra.");
        
        // Esperar a soltar botón para no sacar ráfagas
        while(Orbito.System.getButtonStatus()) delay(50);

    }

}