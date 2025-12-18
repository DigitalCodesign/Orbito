#include "SPIHandler.h"

SemaphoreHandle_t SPIHandler::_safety_block_spi = NULL;

/**
 * @brief Constructor
 * @param spi_bus Pointer to the SPI bus.
 * @param cs_pin GPIO with the Chip Select of the device
 * @param frecuency Hz for the SPI transmission
 */
SPIHandler::SPIHandler(SPIClass* spi_bus, int cs_pin, uint32_t frecuency)
    : _spi(spi_bus), _cs_pin(cs_pin) {
        _settings = SPISettings(frecuency, MSBFIRST, SPI_MODE0);
}

// Initialize hardware
void SPIHandler::begin()
{
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
    // Initialize the mutex blocker one time
    if (_safety_block_spi == NULL)
        _safety_block_spi = xSemaphoreCreateMutex();
}

// Starts a secure transaction (Blocks the bus for other devices)
void SPIHandler::startTransaction()
{
    // Wait the blocking key
    xSemaphoreTake(_safety_block_spi, portMAX_DELAY);
    // Config the bus with the device frequency
    _spi->beginTransaction(_settings);
    // Select the slave
    digitalWrite(_cs_pin, LOW);
}

// Ends a transaction (Free the bus for other devices)
void SPIHandler::endTransaction()
{
    // Deselect the slave
    digitalWrite(_cs_pin, HIGH);
    // Free the configuration
    _spi->endTransaction();
    // Return the blocking key
    xSemaphoreGive(_safety_block_spi);
}

// Send a simple byte
void SPIHandler::spiWrite(uint8_t data)
{
    _spi->transfer(data);
}

// Send a buffer
void SPIHandler::spiWriteBytes(const uint8_t* data, size_t size)
{
    _spi->transferBytes(data, NULL, size);
}

// Reads from a byte
uint8_t SPIHandler::spiRead()
{
    return _spi->transfer(0x00);
}
