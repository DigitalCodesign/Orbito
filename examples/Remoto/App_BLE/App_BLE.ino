#include <Orbito.h>

// Variable global para el sensor (debe ser tipo float para la librería)
float tiempo_activo = 0.0;

// Callback para el interruptor de la App
void onSwitch(bool state) {

  // Borramos zona central
  Orbito.Display.fillCircle(160, 120, 60, 0x0000); 

  if (state) {

      // Estado ON: Círculo Verde y Texto
      Orbito.Display.fillCircle(160, 120, 50, 0x07E0); // Verde
      Orbito.Display.consoleLog("APP: ENCENDIDO");

  } else {

      // Estado OFF: Círculo Rojo y Texto
      Orbito.Display.fillCircle(160, 120, 50, 0xF800); // Rojo
      Orbito.Display.consoleLog("APP: APAGADO");

  }

}

void setup() {

  Orbito.begin();
  Orbito.Display.consoleLog("--- BLE DASHBOARD ---");
  Orbito.Display.consoleLog("Busca 'Orbito_Bot'...");

  // 1. Iniciar el servicio BLE
  Orbito.Remote.initDashboard("Orbito_Bot");

  // 2. Añadir CONTROLES (Interruptores)
  // El primer parámetro es el nombre que aparecerá en la App
  Orbito.Remote.addSwitch("Luz Principal", onSwitch);

  // 3. Añadir SENSORES (Variables en vivo)
  // Vinculamos la variable 'tiempo_activo' para que se envíe sola
  Orbito.Remote.addSensor("Tiempo (s)", &tiempo_activo);
  
  // Estado inicial visual (OFF)
  Orbito.Display.fillCircle(160, 120, 50, 0xF800); 

}

void loop() {

  // Actualizamos la variable del sensor
  // La librería detectará el cambio y lo enviará al móvil automáticamente
  tiempo_activo = millis() / 1000.0;

  // Mantenimiento OBLIGATORIO (Gestiona el BLE internamente)
  Orbito.update();

}
