# Librería Orbito Robot

## 1. Introducción
Esta librería ha sido creada con un único objetivo: convertir la programación de robots avanzados en algo tan intuitivo y divertido como un juego.

Sabemos que la robótica puede parecer difícil. Cables, registros, protocolos extraños... ¡Olvida todo eso! Con Orbito, no necesitas ser un experto ingeniero para dar vida a tu máquina.

### ¿Cómo funciona?
La filosofía de Orbito es muy simple: **El robot funciona igual que un ser vivo.**

En lugar de pelearte con chips y configuraciones complejas, la librería organiza todo el hardware del robot en **Módulos Inteligentes** que imitan las funciones biológicas. Tú solo tienes que decirle a cada parte del "cuerpo" qué quieres hacer:

* **System (El Cuerpo):** Controla la energía, el acciones básicas y los reflejos.
* **Vision (Los Ojos):** Permite al robot ver, hacer fotos y transmitir video.
* **Ear (El Oído):** Le da la capacidad de escuchar y grabar sonidos.
* **Display (La Cara):** Muestra información, dibujos y lo que ve la cámara.
* **Action (La Personalidad):** Controla las emociones y expresiones faciales.
* **Brain (El Cerebro):** Usa Inteligencia Artificial para reconocer objetos y entender el entorno.
* **Connect (La Voz al Mundo):** Se conecta a Internet (WiFi) para comunicarse.
* **Remote (El Control):** Permite manejarlo desde tu móvil por Bluetooth.
* **Storage (La Memoria):** Guarda recuerdos y datos para no olvidarlos al apagarse.

Con Orbito, programar es tan fácil como escribir: *"Orbito, ponte feliz y saca una foto"*.

## 2. Primeros Pasos

### Instalación
Instalar Orbito es muy sencillo, ya que está disponible oficialmente en el gestor de Arduino. No necesitas descargar archivos externos.

1.  Abre **Arduino IDE**.
2.  Ve a la barra lateral izquierda y haz clic en el icono de **Librerías** (parece una pila de libros).
3.  En el buscador, escribe: **Orbito**.
4.  Cuando aparezca, pulsa el botón **Instalar**.

¡Y ya está! El IDE descargará y configurará todo automáticamente.

### Tu Primer Programa (El Mínimo Vital)
Para que Orbito funcione, tu código siempre debe cumplir tres reglas de oro: incluir la librería, despertar al robot al principio y mantenerlo activo siempre.

Copia y pega este código básico para probar que todo funciona:

```cpp
// 1. Incluimos las herramientas de Orbito
#include <Orbito.h>

void setup() {
    // 2. Despertamos al robot
    // Esto enciende la pantalla, la cámara y prepara todos los sistemas.
    Orbito.begin();
    
    // Escribimos un saludo en su pantalla para saber que está vivo
    Orbito.Display.consoleLog("¡Hola Mundo!");
}

void loop() {
    // 3. Mantenimiento vital (¡OBLIGATORIO!)
    // Esta línea permite que el robot "respire", gestione el WiFi 
    // y parpadee los ojos automáticamente. Ponla siempre.
    Orbito.update();
}
```

## 3. Referencia de Módulos (API)

Para que sea fácil de recordar, la librería organiza todas las funciones dentro del objeto principal `Orbito`, dividido en sub-módulos lógicos (Cuerpo, Ojos, Oído...).

La estructura siempre es: `Orbito.[MODULO].[FUNCION]();`

---

### Orbito.System (El Cuerpo)
Este módulo es el encargado de las funciones vitales. Controla la gestión de energía (batería), el botón trasero, el zumbador y los puertos de conexión laterales para sensores externos.

#### Funciones Básicas
| Función | Descripción |
| :--- | :--- |
| `System.getButtonStatus()` | Devuelve `true` si el botón trasero del robot está presionado. |
| `System.tone(Hz, ms)` | Emite un sonido por el zumbador. <br>Ej: `Orbito.System.tone(1000, 500);` (1000Hz durante medio segundo). |
| `System.sleep(segundos)` | Apaga casi todo el robot para ahorrar batería y lo despierta automáticamente pasados X segundos. |
| `System.hibernate()` | El robot se "desmaya" (consumo mínimo) y solo despierta si pulsas el botón trasero. |
| `System.restart()` | Reinicia el robot por completo (como si pulsaras el botón Reset). |

#### Puertos de Expansión (Sensores y Actuadores)
Orbito tiene conectores en los laterales para que puedas enchufar luces LED, sensores de distancia, potenciómetros, etc. **¿Sabías que los conectores son totalmente compatibles con todos los productos MentorBit?**

**Nota:** Estos pines no se controlan con el `digitalWrite` normal de Arduino, debes usar las funciones de `Orbito.System`.

| Función | Descripción |
| :--- | :--- |
| `System.pinMode(pin, modo)` | Configura un pin como `INPUT` (Entrada para sensores) u `OUTPUT` (Salida para luces/motores). |
| `System.digitalWrite(pin, valor)` | Si es salida: Enciende (`1` o `HIGH`) o apaga (`0` o `LOW`) el pin. |
| `System.digitalRead(pin)` | Si es entrada: Lee si hay voltaje (`1`) o no (`0`). Ideal para botones externos. |
| `System.analogWrite(pin, valor)` | Escribe una señal PWM (0 a 255) para controlar brillo de LEDs o velocidad de motores. |
| `System.analogRead(pin)` | Lee el valor de un sensor analógico (0 a 1023). Útil para potenciómetros o sensores de luz. |

### Orbito.Vision (Los Ojos)
Si el módulo System es el cuerpo, Vision son los ojos. Este módulo convierte a tu robot en un fotógrafo, una cámara de seguridad o un director de cine.

#### Modos de Cámara (Las Gafas)
Antes de usar la cámara, debes decirle "cómo" quieres que mire. Es como ponerle unas gafas diferentes según la tarea:

* **Modo AI (`MODE_AI`):** Baja resolución pero muy rápido. Ideal para que el cerebro del robot reconozca objetos a toda velocidad.
* **Modo Streaming (`MODE_STREAMING`):** Configuración equilibrada para transmitir video por WiFi.
* **Modo Alta Calidad (`MODE_HIGH_RES`):** Saca fotos grandes y bonitas, pero tarda más en procesarlas.

Para configurarlo:
```cpp
Orbito.Vision.setMode(CameraHandler::MODE_AI);
```

#### Funciones Principales
| Función | Descripción |
| :--- | :--- |
| `Vision.startWebStream()` | Enciende una "televisión" en Internet. Podrás ver lo que ve el robot desde tu móvil u ordenador (ver módulo Connect). |
| `Vision.snapshot()` | El robot toma una foto instantánea y la guarda en su memoria temporal (RAM). |
| `Vision.setEffect(id)` | Aplica filtros de Instagram: `0` (Normal), `1` (Negativo), `2` (B/N), `6` (Sepia)... |

#### ¡Cuidado con la Memoria! (Regla de Oro)
Las fotos ocupan mucho espacio en el cerebro del robot (RAM). Cuando usas *snapshot()*, el robot se queda "sujetando" la foto con las manos. Si intenta hacer otra cosa sin soltar la foto, se le caerá todo y se reiniciará.

Siempre debes liberar la foto cuando termines de usarla:

```cpp
// 1. Tomar la foto
camera_fb_t* foto = Orbito.Vision.snapshot();

// 2. Hacer algo con ella (mostrarla en pantalla, enviarla...)
Orbito.Display.drawSnapshot(foto);

// 3. ¡SOLTARLA! (Obligatorio)
Orbito.Vision.release(foto);
```

### Orbito.Display (La Cara)
La pantalla es la forma que tiene Orbito de comunicarse contigo. Puedes usarla para escribir mensajes, dibujar formas o mostrar las fotos que hace la cámara.

El sistema de coordenadas empieza arriba a la izquierda `(0, 0)` y termina abajo a la derecha `(240, 320)`.

#### Escribir Texto
Orbito tiene dos formas de escribir: modo "Consola" (fácil, como un chat) o modo "Diseñador" (tú eliges posición, color y tamaño).

| Función | Descripción |
| :--- | :--- |
| `Display.consoleLog("Texto")` | Escribe una línea de texto y hace *scroll* automático hacia arriba cuando se llena la pantalla. Ideal para mensajes de estado. |
| `Display.print("Texto")` | Escribe texto exactamente donde esté el cursor. |
| `Display.setCursor(x, y)` | Mueve el cursor a una posición específica para empezar a escribir allí. |
| `Display.setFont(tamaño)` | Cambia el tamaño de la letra: `1` (Pequeña), `2` (Mediana), `3` (Grande)... |
| `Display.setTextColor(color)` | Cambia el color del texto. |

#### Dibujar Formas (Gráficos)
¡Saca tu lado artístico! Puedes pintar píxel a píxel o usar formas geométricas.

**Nota sobre colores:** Los colores se escriben en formato hexadecimal (HEX). Ejemplos: `0x0000` (Negro), `0xFFFF` (Blanco), `0xF800` (Rojo), `0x07E0` (Verde), `0x001F` (Azul).

| Función | Descripción |
| :--- | :--- |
| `Display.fillScreen(color)` | Borra todo y pinta la pantalla completa de un color. |
| `Display.drawPixel(x, y, color)` | Pinta un único punto. |
| `Display.drawLine(x1, y1, x2, y2, color)` | Dibuja una línea recta desde el punto 1 al punto 2. |
| `Display.fillCircle(x, y, radio, color)` | Dibuja un círculo relleno. |
| `Display.fillRect(x, y, ancho, alto, color)` | Dibuja un rectángulo relleno. |
| `Display.drawSnapshot(foto)` | Muestra en la pantalla una foto tomada anteriormente con `Vision.snapshot()`. |

### Orbito.Action (La Personalidad)
¡Dale vida a tu robot! Este módulo controla la cara para que Orbito deje de ser una máquina y tenga emociones.

#### Expresiones (Emociones)
Puedes cambiar el estado de ánimo de Orbito instantáneamente. Solo tienes que elegir una de las emociones de la lista:

* `NEUTRAL` (Normal)
* `HAPPY` (Feliz)
* `SAD` (Triste)
* `ANGRY` (Enfadado)
* `SURPRISE` (Sorpresa)
* `SLEEPY` (Con sueño)
* `WORRY` (Preocupado)

**¿Cómo se usan?**
Debes escribir la ruta completa de la emoción. Es un poco largo, pero necesario:

```cpp
// Poner cara feliz
Orbito.Action.setExpression(OrbitoRobot::ActionModule::HAPPY);
```

#### Animaciones y Movimiento
Además de cambiar la cara estática, puedes hacer que se mueva para que parezca vivo.

| Función | Descripción |
| :--- | :--- |
| `Action.animateEyes(true)` | Activa el parpadeo automático. El robot cerrará y abrirá los ojos cada cierto tiempo de forma natural. Para que funcione, recuerda poner `Orbito.update()` en el `loop`. |
| `Action.blink()` | Fuerza un parpadeo inmediato (cierra y abre los ojos). |
| `Action.lookAt(x, y)` | Mueve las pupilas manualmente. <br>`x`: De izquierda (-15) a derecha (15). <br>`y`: De arriba (-15) a abajo (15). <br>El centro es `(0, 0)`. |

### Orbito.Brain (El Cerebro IA)
Aquí es donde ocurre la magia. Este módulo conecta tu robot con modelos de **Inteligencia Artificial** (Machine Learning) entrenados en **Edge Impulse**.

El cerebro de Orbito es flexible: puede reconocer objetos (visión), identificar palabras clave (audio) o detectar movimientos y gestos (sensores), dependiendo del modelo que le cargues.

#### Funciones Principales
| Función | Descripción |
| :--- | :--- |
| `Brain.begin()` | Inicializa el sistema de inteligencia artificial. |
| `Brain.setThreshold(0.0 a 1.0)` | Ajusta la "confianza" mínima. Si la IA no está segura (por debajo de este número), devolverá "Unknown". |
| `Brain.predict(foto)` | **Para Visión:** Analiza una foto de la cámara y devuelve el nombre del objeto. |
| `Brain.predict(datos, tamaño)` | **Para Datos/Audio:** Analiza una lista de números (`float*`). Útil para clasificar gestos, sonidos o datos de sensores. |

#### Ejemplo 1: Reconocedor de Objetos (Visión)
```cpp
// 1. Tomar foto rápida
Orbito.Vision.setMode(CameraHandler::MODE_AI);
camera_fb_t* foto = Orbito.Vision.snapshot();

// 2. Analizar
String resultado = Orbito.Brain.predict(foto);
Orbito.Display.consoleLog("Veo: " + resultado);

// 3. Liberar memoria
Orbito.Vision.release(foto);
```

#### Ejemplo 2: Clasificador de Datos (Gestos/Sensores)
Imagina que tienes un modelo que detecta si el robot está "Quieto" o "Moviéndose" basándose en 3 lecturas de un sensor.
```cpp
// Datos de entrada (Ej: lecturas X, Y, Z de un acelerómetro)
float datos_sensor[] = {0.1, 0.5, 9.8}; 

// Preguntar al cerebro (le pasamos los datos y cuántos son)
// sizeof(datos_sensor) / sizeof(float) calcula automáticamente la cantidad (3)
String estado = Orbito.Brain.predict(datos_sensor, 3);

if (estado == "Moviendose") {
    Orbito.Display.consoleLog("¡Terremoto!");
}
```

### Orbito.Connect (Internet y Web)
Este módulo permite que Orbito deje de estar aislado y se conecte al mundo. Puede conectarse al WiFi de tu casa o crear su propia red para que te conectes a él en medio del campo.

Además, ¡Orbito tiene un pequeño servidor web dentro! Puede alojar una página web para que lo controles desde el navegador de tu móvil o PC.

#### 1. Conexión WiFi
Lo primero es estar conectado. Tienes dos opciones:

| Función | Descripción |
| :--- | :--- |
| `Connect.connect("WiFi", "Clave")` | **Modo Estación:** Se conecta al router de tu casa o instituto. |
| `Connect.createAP("NombreRobot")` | **Modo Access Point:** El robot crea su propia red WiFi. Tú buscas esa red en tu móvil y te conectas a ella. Ideal si no hay routers cerca. |
| `Connect.getIP()` | Te dice la dirección IP del robot (ej: `192.168.1.50`). Necesitas saber este número para entrar en su web. |

#### 2. Tu Propia Página Web (Servidor)
Orbito puede guardar código HTML y mostrártelo cuando entres a su IP.

| Función | Descripción |
| :--- | :--- |
| `Connect.startWebServer()` | Enciende el servidor. A partir de aquí, si escribes la IP del robot en Chrome, verás su página. |
| `Connect.setWebInterface(html)` | Carga el código de la página web. Puedes escribir el HTML en una variable de texto (String) y pasársela. |
| `Connect.setWebStatus("Texto")` | Envía un mensaje a la página web. Útil para que el usuario sepa qué está haciendo el robot ("Durmiendo", "Buscando"...). |
| `Connect.onWebCommand(funcion)` | **La más importante.** Define qué función de tu código se ejecutará cuando pulses un botón en la página web. |

#### Ejemplo: Robot con Web
```cpp
// Tu página web básica (HTML)
String miWeb = "<h1>Hola</h1> <button onclick='fetch(\"/cmd?id=saluda\")'>Saluda</button>";

void setup() {
    Orbito.begin();
    
    // 1. Conectar WiFi
    Orbito.Connect.connect("MiCasa", "12345678");
    
    // 2. Configurar la web
    Orbito.Connect.setWebInterface(miWeb);
    Orbito.Connect.onWebCommand(gestionarWeb); // ¿Quién atiende las peticiones?
    Orbito.Connect.startWebServer();
    
    Orbito.Display.consoleLog(Orbito.Connect.getIP()); // Muestra la IP para saber dónde entrar
}

// Esta función se ejecuta cuando alguien pulsa el botón en la web
void gestionarWeb(String comando) {
    if (comando == "saluda") {
        Orbito.Display.consoleLog("¡Hola desde la web!");
    }
}
```

### Orbito.Remote (Control Bluetooth)
¿Quieres controlar tu robot desde el móvil pero no sabes crear Apps? No hay problema.
Este módulo permite usar aplicaciones genéricas de Bluetooth (como *Serial Bluetooth Terminal*) para crear un panel de mandos sin escribir ni una línea de código en el móvil.

#### Funciones Principales
| Función | Descripción |
| :--- | :--- |
| `Remote.initDashboard("Nombre")` | Enciende el Bluetooth. Tu robot aparecerá en el móvil con este nombre. |
| `Remote.addSwitch("Etiqueta", funcion)` | Crea un interruptor en la App. Cuando lo actives en el móvil, el robot ejecutará la función que le digas. |
| `Remote.addSensor("Etiqueta", &variable)` | Muestra en el móvil el valor de una variable del robot en tiempo real (batería, temperatura, contador...). |
| `Remote.log("Texto")` | Envía mensajes de texto al móvil. |

#### Ejemplo: Panel de Control
Imagina que quieres encender una luz desde el móvil y ver cuánta batería queda.

1.  Descarga una app de Terminal Bluetooth en tu móvil.
2.  Carga este código en Orbito:

```cpp
float voltaje = 0; // Variable que queremos ver en el móvil

void setup() {
    Orbito.begin();
    
    // 1. Iniciamos el Bluetooth
    Orbito.Remote.initDashboard("MiOrbito");
    
    // 2. Creamos un botón llamado "Luz"
    // Cuando lo pulses, se ejecutará la función 'cambiarLuz'
    Orbito.Remote.addSwitch("Luz", cambiarLuz);
    
    // 3. Añadimos un sensor para vigilar la batería
    // El símbolo '&' es necesario para que el robot sepa dónde mirar
    Orbito.Remote.addSensor("Bateria", &voltaje);
}

void loop() {
    Orbito.update(); // Necesario para que el Bluetooth funcione
    
    // Actualizamos la variable (simulada)
    voltaje = Orbito.System.getBatteryLevel();
}

// Esta función se ejecuta al tocar el botón en el móvil
void cambiarLuz(bool estado) {
    if (estado) {
        Orbito.Display.consoleLog("Luz ENCENDIDA");
        Orbito.System.digitalWrite(LED_PIN, HIGH);
    } else {
        Orbito.Display.consoleLog("Luz APAGADA");
        Orbito.System.digitalWrite(LED_PIN, LOW);
    }
}
```

### Orbito.Storage (La Memoria)
La memoria RAM (donde van las fotos y variables) se borra cuando apagas el robot.
El módulo Storage es diferente: es como un **bloc de notas permanente**. Lo que escribas aquí seguirá existiendo mañana, aunque le quites la batería al robot.

**Nota Importante:**
Este sistema es muy sencillo. Tienes un único espacio de **4KB** (4096 letras).
Aunque las funciones te pidan un "nombre de archivo" (ej: `/notas.txt`), **Orbito guarda todo en el mismo sitio**.
* Si guardas un archivo nuevo, **borras el anterior**.
* Solo puedes tener "un archivo" a la vez.

#### Funciones Principales
| Función | Descripción |
| :--- | :--- |
| `Storage.writeFile("/nombre", "Texto")` | Borra lo que hubiera antes y guarda el texto nuevo. |
| `Storage.appendFile("/nombre", "Texto")` | Escribe texto al final de lo que ya existe, sin borrar nada. Ideal para listas o registros (logs). |
| `Storage.readFile("/nombre")` | Devuelve el texto que hay guardado. |
| `Storage.format()` | **¡Peligro!** Borra toda la memoria permanente y la deja en blanco. |

#### Ejemplo: Recordar un nombre
Imagina que quieres que el robot recuerde cómo se llama su dueño aunque se apague.

```cpp
void setup() {
    Orbito.begin();
    
    // 1. ¿Tenemos algo guardado?
    if (Orbito.Storage.exists("/dueño.txt")) {
        // Leemos la memoria
        String nombre = Orbito.Storage.readFile("/dueño.txt");
        Orbito.Display.consoleLog("Hola de nuevo, " + nombre);
    } else {
        // No hay nada, es la primera vez
        Orbito.Display.consoleLog("No te conozco.");
        
        // Guardamos el nombre para la próxima vez
        Orbito.Storage.writeFile("/dueño.txt", "Kevin");
        Orbito.Display.consoleLog("¡Nombre guardado!");
    }
}
```

### Orbito.Ear (El Oído)
¡Orbito puede escuchar! Este módulo permite al robot detectar el nivel de ruido o grabar sonidos para analizarlos.

Por defecto, el oído está apagado para ahorrar batería. Debes encenderlo antes de usarlo.

#### Funciones Principales
| Función | Descripción |
| :--- | :--- |
| `Ear.begin()` | Enciende el micrófono. Ponlo en el `setup()`. |
| `Ear.getVolume()` | Devuelve el nivel de ruido actual del 0 al 100. Es muy rápido y no detiene al robot. Ideal para detectar palmadas, gritos o música. |
| `Ear.capture(ms)` | Graba un clip de audio de `ms` milisegundos. **Nota:** El robot se detendrá (bloqueo) mientras graba y guardará el audio en la RAM. |
| `Ear.release(audio)` | Borra la grabación de la memoria. **¡Obligatorio usarlo después de capturar!** |

#### Ejemplo 1: Interruptor por Aplauso
Usamos `getVolume` para encender una luz cuando haya un ruido fuerte.

```cpp
void loop() {
    Orbito.update();

    // Leemos el volumen (0 a 100)
    int ruido = Orbito.Ear.getVolume();

    // Si el ruido pasa de 50 (una palmada fuerte)
    if (ruido > 50) {
        Orbito.Display.consoleLog("¡CLAP DETECTADO!");
        Orbito.System.tone(1000, 100); // Pitido de confirmación
        delay(500); // Espera pequeña para no detectar el mismo aplauso dos veces
    }
}
```

#### Ejemplo 2: Grabadora de Audio (Uso Avanzado)
Cuando grabas audio, usas mucha memoria RAM. Es como pedir una caja prestada. Si no devuelves la caja (`release`), el almacén se llena y el robot falla.

```cpp
// 1. Grabar 2 segundos (2000 ms)
// El robot se quedará quieto estos 2 segundos
int16_t* mi_audio = Orbito.Ear.capture(2000);

// 2. Comprobar si se grabó bien (si no hay memoria, devuelve NULL)
if (mi_audio != NULL) {
    Orbito.Display.consoleLog("Audio grabado correctamente.");
    
    // Aquí podrías enviar el audio por WiFi o procesarlo con la IA
    
    // 3. ¡IMPORTANTE! Liberar la memoria
    Orbito.Ear.release(mi_audio); 
} else {
    Orbito.Display.consoleLog("Error: Memoria llena.");
}
```

## 4. Solución de Problemas Frecuentes

¿Tu robot hace cosas raras? No te preocupes, el 90% de las veces es uno de estos problemas comunes.

### 1. El robot se reinicia solo (Crash)
**Síntoma:** El robot empieza a funcionar, pero de repente se apaga y vuelve a salir el logo de inicio, o la pantalla se pone en blanco.
* **Causa A (Batería):** Los motores y la cámara consumen mucha energía. Si la batería está baja, el voltaje cae y el cerebro se apaga.
    * **Solución:** Carga el robot al 100%.
* **Causa B (Memoria RAM):** Has tomado una foto (`snapshot`) o grabado audio (`capture`) y has olvidado usar `release()`. La memoria se ha llenado y el robot ha colapsado.
    * **Solución:** Revisa tu código y asegúrate de liberar siempre la memoria.

### 2. El robot se queda "congelado" y no parpadea
**Síntoma:** El robot deja de responder, los ojos no se mueven y si tienes WiFi, te desconecta.
* **Causa:** Estás usando `delay(tiempo)` demasiado largos (ej: 5 segundos). Mientras el robot está en un `delay`, su cerebro está en pausa total: no puede gestionar el WiFi, ni mover la cara, ni escuchar botones.
* **Solución:** Evita pausas largas. Usa contadores de tiempo o llama a `Orbito.update()` frecuentemente.

### 3. Error: "Camera Init Failed"
**Síntoma:** Al encenderse, sale un mensaje de error diciendo que la cámara no funciona.
* **Causa:** El cable plano de la cámara (flex) es muy delicado y puede haberse soltado con un golpe.
* **Solución:** Apaga el robot. Presiona suavemente el conector de la cámara para asegurarte de que hace buen contacto.

### 4. El WiFi no conecta
**Síntoma:** Se queda eternamente en "Conectando...".
* **Causa:** Probablemente estás intentando conectarte a una red 5GHz. El chip ESP32 de Orbito solo entiende redes **2.4GHz** (las normales).
* **Solución:** Asegúrate de usar una red 2.4GHz y que la contraseña esté bien escrita.

### 5. Error al subir el código
**Síntoma:** Arduino IDE dice "A fatal error occurred: Failed to connect to ESP32".
* **Causa:** El ordenador no detecta al robot.
* **Solución:**
    1.  Prueba con otro cable USB (algunos cables son solo de carga y no sirven para datos).
    2.  Asegúrate de que el robot está encendido.
    3.  En Arduino, comprueba que en **Herramientas > Puerto** has seleccionado el correcto.
