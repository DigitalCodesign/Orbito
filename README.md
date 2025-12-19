# Librería Orbito Robot

## 1. Introducción
La librería Orbito unifica el control de todo el hardware del robot (Cámara, Pantalla, Motores, IA, Sensores, WiFi, BLE) en un único objeto global llamado Orbito.

Ya no es necesario gestionar múltiples librerías complejas. Todo está organizado en Módulos lógicos (Cuerpo, Ojos, Cerebro, Conectividad, etc.).

### Estructura General
El objeto principal se divide en 8 submódulos:
- System: Energía, Sueño profundo, GPIO expandido, Buzzer.
- Vision: Cámara, Streaming de video, Fotos.
- Display: Gráficos en pantalla, Texto, Dibujo.
- Action: Emociones, Animación de ojos, Boca.
- Brain: Inteligencia Artificial (Detección de objetos/audio).
- Storage: Sistema de archivos (Flash interna).
- Connect: WiFi, Hora (NTP), Dashboard Web.
- Remote: Bluetooth (App móvil) y NFC.

## 2. Primeros Pasos
### Instalación
Asegúrate de incluir la librería al inicio de tu sketch:
```c++
#include <Orbito.h>
```

### El Ciclo de Vida (Setup & Loop)
Para que el robot funcione, debes inicializarlo en el <code>setup()</code> y mantenerlo vivo en el <code>loop()</code>.
```c++
void setup() {
    // Inicializa todos los sistemas (Pantalla, Cámara, Motores, etc.)
    bool ok = Orbito.begin();
    
    if (ok) {
        Orbito.Display.consoleLog("Sistema OK");
    } else {
        // Hubo un error crítico de hardware
    }
}

void loop() {
    // Mantiene WiFi, BLE y Animaciones funcionando.
    // ¡Es obligatorio llamar a esto frecuentemente!
    Orbito.update(); 
}
```

## 3. Referencia de Módulos (API)
### Orbito.System (El Cuerpo)
Controla la energía y los pines de expansión (gestionados por el coprocesador ATtiny).

| Función                                        | Descripción                                                                         |
|------------------------------------------------|-------------------------------------------------------------------------------------|
| <code>System.begin()</code>                    | Arranca los buses internos. Se llama automáticamente en <code>Orbito.begin()</code> |
| <code>System.sleep(X)</code>                   | Apaga todo y duerme el robot por X segundos (Ahorro extremo de batería).            |
| <code>System.hibernate(pin, nivel)</code>      | Duerme indefinidamente hasta que se pulse un botón o sensor externo.                |
| <code>System.getButtonStatus()</code>          | Devuelve <code>true</code> si el botón de usuario está presionado.                  |
| <code>System.tone(frequencia, duracion)</code> | Emite un sonido por el zumbador.                                                    |

Control de Pines Expandidos: Nota: Estas funciones controlan los pines del puerto de expansión, no los del ESP32 directo.

- <code>System.pinMode(pin, mode)</code>
- <code>System.digitalWrite(pin, value)</code>
- <code>System.analogRead(pin)</code>

### Orbito.Vision (Los Ojos)
Manejo de la cámara integrada.

| Función                                    | Descripción                                                           |
|--------------------------------------------|-----------------------------------------------------------------------|
| <code>Vision.startWebStream()</code>       | Inicia un servidor de video en el puerto 81. Accesible vía navegador. |
| <code>Vision.stopWebStream()</code>        | Detiene el vídeo para liberar memoria.                                |
| <code>Vision.snapshot()</code>             | Toma una foto y devuelve un puntero <code>camera_fb_t*</code>.        |
| <code>Vision.saveSnapshot("/f.jpg")</code> | Toma una foto y la guarda directamente en la memoria flash.           |

Gestión de Memoria: Si usas <code>snapshot()</code>, debes liberar la memoria manualmente después de usar la imagen:
```c++
camera_fb_t* foto = Orbito.Vision.snapshot();
// ... procesar foto ...
Orbito.Vision.release(foto); // ¡Importante!
```

### Orbito.Display (La Cara - Gráficos)
Permite dibujar en la pantalla LCD. Usa la sintaxis estándar de Adafruit GFX.

- Básicos: <code>fillScreen(color)</code>, <code>drawPixel(x,y,c)</code>, <code>drawLine(...)</code>, <code>drawRect(...)</code>, <code>drawCircle(...)</code>.
- Texto: <code>setCursor(x,y)</code>, <code>setTextColor(c)</code>, <code>setFont(size)</code>, <code>print("Texto")</code>.
- Avanzado:
    - <code>drawSnapshot(fb)</code>: Dibuja una foto de la cámara en la pantalla.
    - <code>turnOff()</code> / <code>turnOn()</code>: Apaga/Enciende la retroiluminación.

### Orbito.Action (Personalidad y Emociones)
Controla las expresiones faciales predefinidas.

Enumeración de Emociones: Para usar las emociones, debes acceder a través de la clase: <code>OrbitoRobot::ActionModule::[EMOCION]</code>

<code>NEUTRAL</code>, <code>HAPPY</code>, <code>SAD</code>, <code>ANGRY</code>, <code>SURPRISE</code>, <code>SLEEPY</code>, <code>WORRY</code>.

| Función                                     | Descripción                                                      |
|---------------------------------------------|------------------------------------------------------------------|
| <code>Action.setExpression(emocion)</code>  | Cambia la cara actual.                                           |
| <code>Action.animateEyes(true/false)</code> | Activa el parpadeo automático y movimiento aleatorio de pupilas. |
| <code>Action.lookAt(x, y)</code>            | Mueve las pupilas a una coordenada relativa (0,0 es centro).     |
| <code>Action.blink()</code>                 | Fuerza un parpadeo inmediato.                                    |

### Orbito.Brain (Inteligencia Artificial)
Motor de inferencia para conectar modelos de Edge Impulse.

- <code>Brain.load(miAdaptadorIA)</code>: Carga el modelo de IA.
- <code>Brain.predict(imagen)</code>: Ejecuta detección de objetos en una foto.
- <code>Brain.predict(datos, len)</code>: Ejecuta clasificación sobre datos (audio/gestos).

### Orbito.Connect (WiFi y Web)
Gestión de red y panel de control web.

- <code>Connect.connect(ssid, pass)</code>: Se conecta al WiFi.
- <code>Connect.createAP(ssid)</code>: Crea su propia red WiFi.
- <code>Connect.checkUpdates()</code>: Obligatorio en el loop para gestionar actualizaciones OTA.
- <code>Connect.setWebStatus("Mensaje")</code>: Envía texto al panel web de depuración.

### Orbito.Remote (App y Accesorios)
Interacción con el mundo exterior.

Bluetooth (App Móvil):

- <code>Remote.initDashboard("NombreRobot")</code>: Inicia el servicio BLE.
- <code>Remote.addSwitch("Luz", callback)</code>: Crea un botón en la App.
- <code>Remote.addSensor("Temp", &variable)</code>: Envía el valor de una variable en tiempo real a la App.

NFC (Etiquetas):

- <code>Remote.readTagText()</code>: Lee el contenido de una tarjeta NFC cercana.
- <code>Remote.writeUrl("https://...")</code>: Graba una dirección web en una tarjeta.

### Orbito.Storage (Memoria Flash)
Sistema de archivos para guardar configuraciones o logs.

- <code>Storage.writeFile("/log.txt", "Hola")</code>: Crea/Sobrescribe un archivo.
- <code>Storage.appendFile("/log.txt", " Mas texto")</code>: Añade al final.
- <code>Storage.readFile("/log.txt")</code>: Lee el contenido.
- <code>Storage.listDir()</code>: Muestra todos los archivos en la memoria.

## 4. Solución de Problemas Frecuentes
1. Error: <code>Orbito.Action.Emotion</code> no existe.
    - Causa: Estás intentando acceder a un Tipo desde una Instancia.
    - Solución: Usa <code>OrbitoRobot::ActionModule::[EMOCION]</code>.

2. El robot se reinicia al tomar fotos.
    - Causa: Falta de memoria RAM o "Brownout" (bajada de voltaje).
    - Solución: Baja la calidad con <code>Orbito.Vision.setResolution(QVGA)</code> o asegura una buena fuente de alimentación.

3. Las animaciones de los ojos van a tirones.
    - Causa: Tienes código bloqueante (<code>delay()</code>) en tu <code>loop(</code>).
    - Solución: Evita usar <code>delay()</code>. Usa contadores con <code>millis()</code> y llama siempre a <code>Orbito.update()</code> en cada ciclo.

4. No puedo subir código nuevo (Puerto ocupado).
    - Causa: Si el robot está en Deep Sleep, el USB puede desconectarse.
    - Solución: Mantén pulsado el botón <code>BOOT</code> del ESP32 mientras conectas el cable USB para forzar el modo de programación.