#ifndef PORT_HANDLER_H
#define PORT_HANDLER_H

/**
 * HEX  | Command       | Params (ESP)         | Response (ATtiny)
 * 0x01 | PIN_MODE      | [PIN, MODE]          | [ACK]
 * 0x02 | DIGITAL_WRITE | [PIN, VAL]           | [ACK]
 * 0x03 | DIGITAL_READ  | [PIN]                | [ACK]
 * 0x10 | I2C_WRITE     | [ADDR, LEN, DATA...] | [ACK]
 * 0x11 | I2C_READ      | [ADDR, LEN]          | [ACK]
 * 0x20 | SPI_INIT      | [MODE, CLK_DIV]      | [ACK]
 * 0x21 | SPI_TRANSFER  | [LEN, DATA...]       | [DATA...]
 * 0x99 | PING          | []                   | [0xAA]
 */

#include <Arduino.h>

#define TINY_SPI_MODE_0 0x00
#define TINY_SPI_MODE_1 0x01
#define TINY_SPI_MODE_2 0x02
#define TINY_SPI_MODE_3 0x03

// SPI clock dividers
#define TINY_SPI_DIV2  2
#define TINY_SPI_DIV4  4
#define TINY_SPI_DIV8  8
#define TINY_SPI_DIV16 16

// Command codes
#define TINY_START      0xAA
#define TINY_CMD_PMODE  0x01
#define TINY_CMD_DWRITE 0x02
#define TINY_CMD_DREAD  0x03
#define TINY_CMD_AWRITE 0x04
#define TINY_CMD_AREAD  0x05
#define TINY_CMD_I2C_W  0x10
#define TINY_CMD_I2C_R  0x11
#define TINY_CMD_SPI_I  0x20
#define TINY_CMD_SPI_T  0x21
#define TINY_CMD_SLEEP_MONITOR 0x90
#define TINY_CMD_PING   0x99

// Configuration params
#define TINY_SERIAL_RX_PIN 44
#define TINY_SERIAL_TX_PIN 43
#define TINY_SERIAL_BAUDRATE 115200

class PortHandler {

    public:

        /**
         * @brief Constructor
         * @param serial_ref Reference to the hardware serial port.
         */
        PortHandler(HardwareSerial& serial_ref);

        /**
         * @brief Starts the communication.
         */
        bool begin();

        // --- GPIO CONTROL
        /**
         * @brief Send command to configure a GPIO working mode.
         * @param pin GPIO number.
         * @param mode Working mode for the GPIO.
         */
        void pinMode(uint8_t pin, uint8_t mode);

        /**
         * @brief Send command to change the state of a digital output.
         * @param pin GPIO number configured as digital output.
         * @param value State to set the GPIO (1 or 0, HIGH or LOW)
         */
        void digitalWrite(uint8_t pin, uint8_t value);

        /**
         * @brief Send command to change a PWM value generated in an analog output.
         * @param pin GPIO number configured as analog output.
         * @param value Value to set the PWM (from 0 to 255).
         */
        void analogWrite(uint8_t pin, uint8_t value);

        /**
         * @brief Send command to read the actual state of a digital input.
         * @param pin GPIO number configured as digital input.
         * @return An integer (1 or 0, HIGH or LOW).
         */
        int digitalRead(uint8_t pin);

        /**
         * @brief Send command to read the actual analog value of an analog input.
         * @param pin GPIO number configured as analog input.
         * @return An integer from 0 to 4095 (0 to 3'3V).
         */
        int analogRead(uint8_t pin);

        // --- I2C CONTROL
        /**
         * @brief Send command to write in the I2C bus.
         * @param address Objetive device I2C address.
         * @param data Pointer to the data to be sent.
         * @param len Length of the data to be sent.
         * @return True if the transmission was successful.
         */
        bool i2cWrite(uint8_t address, const uint8_t* data, size_t len);

        /**
         * @brief Send command to read from the I2C bus.
         * @param address Objetive device I2C address.
         * @param buffer Pointer to the buffer to store the data readen.
         * @param len Length of the data to be readen.
         * @return True if the transmission was successful.
         */
        bool i2cRead(uint8_t address, uint8_t* buffer, size_t len);

        // --- SPI CONTROL
        /**
         * @brief Configure and start the SPI bus.
         * @param mode SPI working mode.
         * @param clock_divider SPI bus clock divider
         */
        void spiBegin(uint8_t mode = TINY_SPI_MODE_0, uint8_t clock_divider = TINY_SPI_DIV4);

        /**
         * @brief Transfer data to the SPI bus.
         * @param buffer Pointer to the data to be sent.
         * @param len Length of the data to be sent.
         */
        void spiTransfer(uint8_t* buffer, size_t len);

        /**
         * @brief Sends a byte and received the response.
         * @param data Byte to be sent.
         * @return The response byte.
         * 
         */
        uint8_t spiTransfer(uint8_t data);

        /**
         * @brief Create in the ATtiny a watchdog over a GPIO.
         * @param pin GPIO where install the watchdog.
         * @param level Voltage level that must trigger the watchdog.
         */
        void triggerRemoteSleep(uint8_t pin, uint8_t level);

    private:

        HardwareSerial& _serial;
        const uint32_t _timeout = 200;

        // Helper function to send a payload to the ATtiny.
        void _sendPacket(uint8_t cmd, const uint8_t* payload, size_t len);

        // Helper function to read a response from the ATtiny.
        bool _readResponse(uint8_t* buffer, size_t expected_len);

        // Helper function to calculate the CRC of a payload.
        uint8_t _calcCRC(uint8_t cmd, const uint8_t* data, size_t len);

};

#endif
