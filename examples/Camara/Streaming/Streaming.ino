#include <Orbito.h>

// --- TUS CREDENCIALES ---
const char* SSID = "TU_WIFI";
const char* PASS = "TU_CLAVE";

void setup() {

    // 1. INICIAR EL MONITOR Y EL ROBOT
    Serial.begin(115200);
    Orbito.begin();

    Orbito.Display.consoleLog("Iniciando...");

    // 2. Conectar WiFi
    // La función connect es bloqueante, espera a que termine
    Orbito.Display.consoleLog("Conectando WiFi...");
    Orbito.Connect.connect(SSID, PASS);

    // Esperamos mientras la IP sea inválida (0.0.0.0)
    int intentos = 0;
    while (Orbito.Connect.getIP() == "0.0.0.0") {

        delay(500);

        // Feedback visual simple (un punto)
        if (Serial) Serial.print("."); 

        // Timeout de seguridad (15 segundos aprox)
        intentos++;

        if (intentos > 30) {
            Orbito.Display.consoleLog("ERROR: Tiempo agotado.");
            Orbito.Display.consoleLog("Revisa la clave WiFi.");
            while(1) delay(100); // Detener ejecución
        }

    }

    // 3. Iniciar el Servidor Web
    Orbito.Vision.setMode(CameraHandler::MODE_STREAMING);
    // No hace falta setResolution, el MODE_STREAMING ya pone QVGA por defecto
    Orbito.Vision.startWebStream(); 

    // 4. Mostrar la dirección
    Orbito.Display.fillScreen(0x0000); // Limpiar pantalla
    Orbito.Display.consoleLog("VIDEO ACTIVO");
    Orbito.Display.consoleLog("IP: " + Orbito.Connect.getIP());
    
    // IMPORTANTE: La ruta del video es /stream
    Orbito.Display.consoleLog("Ruta: :81/stream");

}

void loop() {

    // Mantiene el WiFi y el servidor activos
    Orbito.update();

}
