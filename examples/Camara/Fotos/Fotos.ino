#include <Orbito.h>

void setup() {
    Serial.begin(115200);
    
    // Espera de seguridad para el USB
    while (!Serial && millis() < 2000) delay(10);

    // Iniciar el sistema completo
    Orbito.begin();
    
    // 1. CONFIGURACIÓN
    // Ponemos la cámara en modo 'AI' (RGB565).
    // Tu función drawSnapshot requiere RGB565, así que esto es obligatorio.
    Orbito.Vision.setMode(CameraHandler::MODE_AI);

    Orbito.Display.consoleLog("Listo.\nPulsa boton para FOTO");
    Serial.println("Sistema iniciado en Modo Pantalla (RGB565).");
}

void loop() {
    // Mantenimiento de tareas de fondo
    Orbito.update();

    // Al pulsar el botón...
    if (Orbito.System.getButtonStatus()) {
        
        // Limpiamos la pantalla (opcional)
        Orbito.Display.fillScreen(0x0000); 

        // 2. CAPTURAR
        // Obtenemos la foto cruda (RGB565)
        camera_fb_t* frame = Orbito.Vision.snapshot();

        if (frame) {
            Serial.printf("Captura OK. %u bytes.\n", frame->len);

            // 3. DIBUJAR
            // ¡Esta es la magia! Usamos tu función nativa.
            // Ella sola se encarga de intercambiar los bytes de color y pintar.
            Orbito.Display.drawSnapshot(frame);

            // 4. LIBERAR MEMORIA
            // Vital para no bloquear la cámara en la siguiente foto
            Orbito.Vision.release(frame);
            
        } else {
            Orbito.Display.consoleLog("Error: Foto nula");
            Serial.println("Error: snapshot devolvió NULL");
        }

        // Pequeño debounce y espera para ver la foto
        delay(2000);
        Orbito.Display.consoleLog("Listo para otra.");
        
        // Esperamos a que el usuario suelte el botón
        while(Orbito.System.getButtonStatus()) delay(10);
    }
}
