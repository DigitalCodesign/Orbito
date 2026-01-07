#include <Orbito.h>

void setup() {
    Orbito.begin();
    Orbito.Action.setExpression(OrbitoRobot::ActionModule::NEUTRAL);
}

void loop() {
    Orbito.update();

    // Mirar Izquierda
    Orbito.Action.lookAt(-15, 0); 
    delay(1000);

    // Mirar Derecha
    Orbito.Action.lookAt(15, 0); 
    delay(1000);

    // Mirar Arriba
    Orbito.Action.lookAt(0, -15); 
    delay(1000);

    // Centro y Parpadear
    Orbito.Action.lookAt(0, 0);
    Orbito.Action.blink();
    delay(1000);
}