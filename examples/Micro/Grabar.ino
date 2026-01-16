#include <Orbito.h>

// Configuración
const int DURACION_GRABACION = 2000; // 2 segundos (Cuidado con la RAM, no se puede aumentar demasiado)

void setup() {

  // Inicializar Serial rápido para transmitir audio
  Serial.begin(115200); 

  Orbito.begin();
  Orbito.Ear.begin(); // Encender micrófono

  Orbito.Display.consoleLog("Listo para grabar");
  Orbito.Display.consoleLog("Pulsa el boton...");

}

void loop() {

  Orbito.update();

  if (Orbito.System.getButtonStatus()) {

    // 1. Feedback visual
    Orbito.Display.fillScreen(0xF800); // Pantalla Roja (Grabando)
    Orbito.Display.setCursor(80, 140);
    Orbito.Display.setTextColor(0xFFFF);
    Orbito.Display.print("GRABANDO...");

    // 2. CAPTURAR (La librería gestiona la memoria y el hardware)
    int16_t* audio = Orbito.Ear.capture(DURACION_GRABACION);

    // 3. PROCESAR (Aquí el usuario decide qué hacer)
    if (audio != NULL) {

      Orbito.Display.fillScreen(0x0000); // Negro
      Orbito.Display.consoleLog("Enviando USB...");

      // Calculamos cuántas muestras hay: (16000 muestras/seg * ms) / 1000
      size_t muestras = (16000 * DURACION_GRABACION) / 1000;

      // Imprimir en formato CSV (Valor, Valor, Valor...)
      // Este formato es perfecto para copiar a Excel o para Edge Impulse Data Forwarder
      for (int i = 0; i < muestras; i++) {
        Serial.print(audio[i]);
        if (i < muestras - 1) Serial.print(", ");
      }
      Serial.println(); // Salto de línea final

      // 4. LIMPIEZA (Obligatorio)
      Orbito.Ear.release(audio);

      Orbito.Display.consoleLog("Hecho.");

    } else {
      Orbito.Display.consoleLog("Error: Falta RAM");
    }

    // Esperar a soltar botón
    while(Orbito.System.getButtonStatus()) delay(10);
    Orbito.Display.fillScreen(0x0000);
    Orbito.Display.consoleLog("Listo.");

  }

}
