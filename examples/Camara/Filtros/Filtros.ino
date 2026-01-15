#include <Orbito.h>

// Variable para controlar qué filtro estamos usando (0 a 6)
int filtro_actual = 0;
const int MAX_FILTROS = 7; 

// Helper para obtener el nombre del efecto
String obtenerNombreFiltro(int id) {
  switch(id) {
    case 0: return "NORMAL";
    case 1: return "NEGATIVO";
    case 2: return "BLANCO Y NEGRO"; // Grayscale
    case 3: return "ROJIZO";
    case 4: return "VERDOSO";
    case 5: return "AZULADO";
    case 6: return "SEPIA";
    default: return "DESCONOCIDO";
  }
}

void setup() {

  Serial.begin(115200);

  // 1. Iniciar Robot
  Orbito.begin();

  // 2. Configurar Modo Pantalla (RGB565)
  Orbito.Vision.setMode(CameraHandler::MODE_AI);

  Orbito.Display.consoleLog("LISTO");
  Orbito.Display.consoleLog("Pulsa para cambiar");
  Orbito.Display.consoleLog("de filtro y foto.");

}

void loop() {

  Orbito.update();

  if (Orbito.System.getButtonStatus()) {

    // -----------------------------------------------------
    // 1. CAMBIAR EL FILTRO
    // -----------------------------------------------------
    filtro_actual++;
    if (filtro_actual >= MAX_FILTROS) filtro_actual = 0;

    // Aplicamos el efecto en el hardware de la cámara
    Orbito.Vision.setEffect(filtro_actual);

    Orbito.Display.fillScreen(0x0000);
    Orbito.Display.setCursor(10, 100);
    Orbito.Display.consoleLog("Cargando " + obtenerNombreFiltro(filtro_actual) + "...");

    // -----------------------------------------------------
    // 2. LIMPIEZA DE BUFFER (CRUCIAL AL CAMBIAR EFECTOS)
    // -----------------------------------------------------
    // Esperamos un poco y tiramos una foto a la basura.
    delay(300); 
    camera_fb_t* basura = Orbito.Vision.snapshot();
    Orbito.Vision.release(basura);

    // -----------------------------------------------------
    // 3. CAPTURA REAL
    // -----------------------------------------------------
    camera_fb_t* frame = Orbito.Vision.snapshot();

    if (frame) {

      // Dibujar la foto
      Orbito.Display.drawSnapshot(frame);

      // Escribir el nombre del filtro encima de la foto
      // Usamos un truco: Texto negro con fondo blanco para que se lea bien
      Orbito.Display.setTextColor(0x0000); // Texto Negro
      Orbito.Display.setFont(2);

      // Dibujamos un recuadro blanco arriba para el texto
      Orbito.Display.fillRect(0, 0, 320, 30, 0xFFFF); 
      Orbito.Display.setCursor(10, 5);
      Orbito.Display.print("Filtro: " + obtenerNombreFiltro(filtro_actual));

      // Liberar memoria
      Orbito.Vision.release(frame);

    } else {

      Orbito.Display.consoleLog("Error Captura");

    }

    delay(1000); // Ver la foto 1 segundo

    // Restaurar color de texto para el menú
    Orbito.Display.setTextColor(0xFFFF); 

    // Esperar a soltar botón
    while(Orbito.System.getButtonStatus()) delay(50);

  }

}
