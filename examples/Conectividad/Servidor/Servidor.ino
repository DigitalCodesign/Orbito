#include <Orbito.h>

// --- CONFIGURACIÓN WIFI ---
const char* WIFI_SSID = "TU_WIFI";
const char* WIFI_PASS = "TU_CLAVE";

// --- INTERFAZ WEB (HTML + JS) ---
const char* WEB_UI PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; background: #1a1a1a; color: white; padding: 20px; }
    .box { background: #333; padding: 20px; border-radius: 15px; margin: 20px auto; max-width: 400px; }
    h1 { color: #ff9800; }
    .btn { background: #007bff; color: white; border: none; padding: 15px 30px; font-size: 18px; border-radius: 5px; cursor: pointer; margin: 10px; }
    .btn:active { background: #0056b3; }
    .data { font-size: 24px; color: #4caf50; font-weight: bold; }
  </style>
</head>
<body>

  <h1>Orbito Control</h1>

  <div class="box">
    <p>Estado del Robot:</p>
    <div id="status_text" class="data">Cargando...</div>
  </div>

  <div class="box">
    <p>Acciones:</p>
    <button class="btn" onclick="sendCommand('saludar', 1)">👋 Saludar</button>
    <button class="btn" onclick="sendCommand('color', 1)">🎨 Cambiar Color</button>
  </div>

<script>
  function sendCommand(id, val) {
    fetch('/cmd?id=' + id + '&value=' + val);
  }

  setInterval(() => {
    fetch('/status')
      .then(response => response.text())
      .then(text => {
        document.getElementById("status_text").innerText = text;
      });
  }, 1000);
</script>

</body>
</html>
)rawliteral";

// --- CALLBACK DE COMANDOS ---
void procesarComandoWeb(String id, int valor) {
    
  Orbito.Display.consoleLog("CMD: " + id);

  if (id == "saludar") {

    Orbito.Action.setExpression(OrbitoRobot::ActionModule::HAPPY);
    
    // Sonidos
    Orbito.System.tone(1200, 100); 
    delay(150);
    Orbito.System.tone(1800, 200); 
    
    Orbito.Action.blink();
    delay(2000);
    Orbito.Action.setExpression(OrbitoRobot::ActionModule::NEUTRAL);

  } else if (id == "color") {

    uint16_t colorRandom = random(0xFFFF);
    Orbito.Display.fillCircle(160, 120, 50, colorRandom);

  }

}

void setup() {

  // 1. Iniciar Hardware
  Orbito.begin();
  Orbito.Display.consoleLog("Iniciando Web...");

  // 2. Conectar WiFi
  Orbito.Connect.connect(WIFI_SSID, WIFI_PASS);

  while (Orbito.Connect.getIP() == "0.0.0.0") {
    delay(500);
    Orbito.Display.print(".");
  }

  // 3. INICIAR SERVIDOR WEB
  // Ahora usamos la función lógica dentro de Connect
  Orbito.Connect.startWebServer(); 

  // 4. CONFIGURAR LA INTERFAZ Y CALLBACKS
  Orbito.Connect.setWebInterface(WEB_UI);
  Orbito.Connect.onWebCommand(procesarComandoWeb);

  // 5. Mostrar IP
  Orbito.Display.fillScreen(0x0000);
  Orbito.Display.consoleLog("WEB LISTA:");
  Orbito.Display.consoleLog(Orbito.Connect.getIP());

}

void loop() {

  Orbito.update();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {

    lastUpdate = millis();
    // Este texto aparecerá en la web automáticamente
    String estado = "Activo: " + String(millis() / 1000) + "s";
    Orbito.Connect.setWebStatus(estado);

  }

}
