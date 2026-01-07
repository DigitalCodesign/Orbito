#include <Orbito.h>

// Pines del puerto de expansión (No son pines del ESP32, son del ATtiny)
#define PIN_LED_EXT   3 
#define PIN_POTENCIO  4 

void setup() {
    Orbito.begin();

    // Configurar pines del expansor
    Orbito.System.pinMode(PIN_LED_EXT, OUTPUT);
}

void loop() {
    Orbito.update();

    // Leer valor analógico (0-255 o similar según config ATtiny)
    int val = Orbito.System.analogRead(PIN_POTENCIO);
    Orbito.Display.consoleLog("Sensor: " + String(val));

    // Parpadear LED externo basado en la lectura
    Orbito.System.digitalWrite(PIN_LED_EXT, true);
    delay(100);
    Orbito.System.digitalWrite(PIN_LED_EXT, false);
    delay(val); // El retardo depende del sensor
}
