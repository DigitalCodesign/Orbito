#ifndef MIC_HANDLER_H
#define MIC_HANDLER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "../config/Pinout.h"

// Default configuration for voice
#define MIC_SAMPLE_RATE 16000
#define MIC_I2S_PORT I2S_NUM_0

class MicHandler {

    public:

        /**
         * @brief Constructor
         */
        MicHandler();

        /**
         * @brief Initialize I2S hardware in PDM mode.
         * @return True when initialization was OK, False otherwise.
         */
        bool begin();

        /**
         * @brief Stops the I2S to free resources.
         */
        void end();

        /**
         * @brief Read raw audio samples.
         * @param buffer Array to store data.
         * @param samples Number of samples to be readen.
         * @return Number of samples stored in the array.
         */
        size_t read(int16_t* buffer, size_t samples);

    private:

        bool _initialized;

};

#endif