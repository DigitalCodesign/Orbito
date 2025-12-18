#ifndef AI_INTERFACE_H
#define AI_INTERFACE_H

#include <Arduino.h>
#include "esp_camera.h"

struct AIResult {
    String label;
    float confidence;
    float value;
    bool has_detection;
};

class AIInterface
{

    public:

        virtual ~AIInterface() {}
        virtual AIResult predict(camera_fb_t* frame) = 0;
        virtual AIResult predict(float* data, size_t len) = 0;
        virtual void setThreshold(float t) = 0;

};

#endif
