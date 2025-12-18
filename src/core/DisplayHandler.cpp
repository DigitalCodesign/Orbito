#include "DisplayHandler.h"

/**
 * @brief Constructor
 * @param spi_bus Pointer to the shared SPI bus.
 */
DisplayHandler::DisplayHandler(SPIClass* spi_bus)
    : SPIHandler(spi_bus, TFT_CS_PIN, 80000000)
{
    _tft = new Adafruit_ST7789(spi_bus, TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
}

/**
 * @brief Initialize the display and the Adafruit Driver
 */
void DisplayHandler::begin()
{
    // Initialize mutex
    SPIHandler::begin();
    // Initialize the Display
    xSemaphoreTake(_safety_block_spi, portMAX_DELAY);
    _tft->init(TFT_WIDTH, TFT_HEIGHT);
    _tft->setRotation(TFT_ROTATION);
    _tft->fillScreen(ST77XX_BLACK);
    xSemaphoreGive(_safety_block_spi);
}

/**
 * @brief Execute commands for drawing safely. Blocks the SPI bus,
 * launch the command and free the bus again.
 * @param drawCallback Lambda function with the instructions to draw.
 */
void DisplayHandler::draw(std::function<void(Adafruit_ST7789&)> drawCallback)
{
    xSemaphoreTake(_safety_block_spi, portMAX_DELAY);
    drawCallback(*_tft);
    xSemaphoreGive(_safety_block_spi);
}

/**
 * @brief Direct access to the TFT object (CAUTION)
 * If this method is used directly, it may cause conflicts with the flash memory
 */
Adafruit_ST7789* DisplayHandler::getDriver()
{
    return _tft;
}
