#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include "./SPIHandler.h"

// Adafruit dependencies for displays
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <functional>

// Display configuration (ST7789 2.4" 240x320 pixels)
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_CS_PIN 46
#define TFT_RST_PIN 45
#define TFT_DC_PIN 10
#define TFT_ROTATION 1

class DisplayHandler : public SPIHandler {

    public:

        /**
         * @brief Constructor
         * @param spi_bus Pointer to the shared SPI bus.
         */
        DisplayHandler(SPIClass* spi_bus);

        /**
         * @brief Initialize the display and the Adafruit Driver
         */
        void begin() override;

        /**
         * @brief Execute commands for drawing safely. Blocks the SPI bus,
         * launch the command and free the bus again.
         * @param drawCallback Lambda function with the instructions to draw.
         */
        void draw(std::function<void(Adafruit_ST7789&)> drawCallback);

        /**
         * @brief Direct access to the TFT object (CAUTION)
         * If this method is used directly, it may cause conflicts with the flash memory
         */
        Adafruit_ST7789* getDriver();

    private:

        // Intern instance for Adafruit Driver
        Adafruit_ST7789* _tft = NULL;

};

#endif
