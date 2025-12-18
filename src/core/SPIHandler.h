#ifndef SPI_HANDLER_H
#define SPI_HANDLER_H

#include <Arduino.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class SPIHandler {

    protected:

        // Global SPI pointer
        SPIClass* _spi;
        // Specific configuration for the device
        SPISettings _settings;
        int _cs_pin;
        // The must be static to be shared by all classes that uses SPI bus
        // If a device close it, other device can't use it until the first device open it.
        static SemaphoreHandle_t _safety_block_spi;

    public:

        /**
         * @brief Constructor
         * @param spi_bus Pointer to the SPI bus.
         * @param cs_pin GPIO with the Chip Select of the device
         * @param frecuency Hz for the SPI transmission
         */
        SPIHandler(SPIClass* spi_bus, int cs_pin, uint32_t frecuency);

        // Initialize hardware
        virtual void begin();

        // Starts a secure transaction (Blocks the bus for other devices)
        void startTransaction();

        // Ends a transaction (Free the bus for other devices)
        void endTransaction();

        // Send a simple byte
        void spiWrite(uint8_t data);

        // Send a buffer
        void spiWriteBytes(const uint8_t* data, size_t size);

        // Reads from a byte
        uint8_t spiRead();

};

#endif
