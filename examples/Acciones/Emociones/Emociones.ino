#include <Orbito.h>

// Array con todas las emociones disponibles
OrbitoRobot::ActionModule::Emotion emotions[] = {
    OrbitoRobot::ActionModule::NEUTRAL,
    OrbitoRobot::ActionModule::HAPPY,
    OrbitoRobot::ActionModule::SAD,
    OrbitoRobot::ActionModule::ANGRY,
    OrbitoRobot::ActionModule::SURPRISE,
    OrbitoRobot::ActionModule::WORRY,
    OrbitoRobot::ActionModule::SLEEPY
};

int index_emo = 0;
unsigned long last_change = 0;

void setup() {
    Orbito.begin();
    Orbito.Action.animateEyes(true); // Activar parpadeo automático
}

void loop() {
    Orbito.update(); // Necesario para la animación suave

    // Cambiar cada 3 segundos
    if (millis() - last_change > 3000) {
        last_change = millis();

        Orbito.Action.setExpression(emotions[index_emo]);

        // Siguiente emoción (cíclico)
        index_emo = (index_emo + 1) % 7;
    }
}
