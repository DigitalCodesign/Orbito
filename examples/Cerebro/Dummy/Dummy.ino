#include <Orbito.h>

// 1. Crear una clase que herede de AIInterface
class MiInteligencia : public AIInterface {
    public:
        // Implementar la función predict obligatoria
        AIResult predict(camera_fb_t* image) override {
            // AQUÍ IRÍA EL CÓDIGO DE EDGE IMPULSE
            // Como es un ejemplo dummy, devolvemos un valor falso simulado

            bool deteccion = true; // Simulamos que detectamos algo
            if (deteccion) {
                return { "Persona", 0.95f, 0.0f, true }; 
            } else {
                return { "Nada", 0.0f, 0.0f, false };
            }
        }

        // Stub para la predicción de audio/sensores (no usada en este ejemplo)
        AIResult predict(float* data, size_t len) override {
        return { "", 0.0f, 0.0f, false };
        }

        void setThreshold(float t) override {}
};

// Instancia de nuestra IA personalizada
MiInteligencia miIA;

void setup() {
    Orbito.begin();

    // Cargar nuestra IA en el cerebro del robot
    Orbito.Brain.load(miIA);
    Orbito.Display.consoleLog("Cerebro Cargado (Sim)");
}

void loop() {
    Orbito.update();

    if (Orbito.System.getButtonStatus()) {
        camera_fb_t* frame = Orbito.Vision.snapshot();

        // Ejecutar predicción
        AIResult res = Orbito.Brain.predict(frame);

        if (res.valid) {
            Orbito.Display.consoleLog("Veo: " + res.label);
            Orbito.Display.consoleLog("Conf: " + String(res.confidence * 100) + "%");
        }

        Orbito.Vision.release(frame);
        delay(1000);
    }
}
