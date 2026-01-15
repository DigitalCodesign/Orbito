#include <Orbito.h>

#define PIN_POTENCIO  PORT_RIGHT_IO_1_1 

void setup() {

  Orbito.begin();

  Orbito.Display.consoleLog("Potenciometro en PB4");
  Orbito.Display.consoleLog("(PORT_RIGHT_IO_1_1)");

  delay(2000); 
  Orbito.Display.fillScreen(0x0000); // Limpiar pantalla

}

void loop() {

  Orbito.update();

  // 1. Leer sensor usando el pin definido
  int lectura = Orbito.System.analogRead(PIN_POTENCIO);

  // 2. Mapear el rango correcto (0-1023) a tiempo de espera (10ms a 200ms)
  int velocidad = map(lectura, 0, 1023, 10, 200);

  // 3. LED VIRTUAL EN PANTALLA

  // ENCENDIDO (Círculo Rojo)
  Orbito.Display.fillCircle(120, 160, 60, 0xF800); 
  delay(velocidad); 

  // APAGADO (Círculo Negro / Borrado)
  Orbito.Display.fillCircle(120, 160, 60, 0x0000); 
  delay(velocidad);

}