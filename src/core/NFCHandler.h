#ifndef NFC_HANDLER_H
#define NFC_HANDLER_H

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief Default I2C Addresses for ST25DV64K.
 * - USER_MEM: Access to the EEPROM data.
 * - SYSTEM_MEM: Access to the configuration and dynamic status registers.
 */
#define ST25_ADDR_USER   0x53
#define ST25_ADDR_SYSTEM 0x57

/**
 * @brief Key Registers.
 */
#define REG_IT_STS_DYN 0x2005 // Dynamic Interrupt Status Register. Bit 0 indicates RF Field presence.
#define REG_IC_REF     0x0017 // IC Reference Register. Validates connection.

/**
 * @brief NDEF URI Prefixes
 * Used to compress common protocols in NDEF records.
 */
#define NDEF_URI_PREFIX_HTTP_WWW  0x01
#define NDEF_URI_PREFIX_HTTPS_WWW 0x02
#define NDEF_URI_PREFIX_HTTP      0x03
#define NDEF_URI_PREFIX_HTTPS     0x04

class NFCHandler
{

    public:

        /**
         * @brief Constructor.
         * @param wire_ref Reference to the I2C bus.
         */
        NFCHandler(TwoWire& wire_ref);

        /**
         * @brief Initializes the module.
         * Checks I2C communication with the System parition.
         * @return True if the chip responds, false otherwise.
         */
        bool begin();

        /**
         * @brief Checks if an RF Field is present.
         * Since GPO pin is not used, thos performs an I2C read to the internal status register.
         * @return True if RF Field is detected.
         */
        bool isRFFieldPresent();

        /**
         * @brief Writes an URL to the tag.
         * @param url The web address.
         * @param prefix Protocol prefix.
         */
        bool writeURI(String url, uint8_t prefix = NDEF_URI_PREFIX_HTTPS);

        // --- RAW DATA ACCESS ---
        void writeBytes(uint16_t mem_addr, uint8_t* data, uint8_t len);
        void readBytes(uint16_t mem_addr, uint8_t* buffer, uint8_t len);

    private:

        TwoWire& _wire;

        // Internal helper to read registers from the System partition
        uint8_t _readRegister(uint8_t device_addr, uint16_t register_addr);

        // Internal helper to write to User Memory
        void _writeUserMemory(uint16_t memory_addr, uint8_t* data, uint8_t len);

};

#endif
