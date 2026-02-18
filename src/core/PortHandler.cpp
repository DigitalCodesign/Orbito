#include "PortHandler.h"

/**
 * @brief Constructor
 * @param serial_ref Reference to the hardware serial port.
 */
PortHandler::PortHandler(HardwareSerial& serial_ref) : _serial(serial_ref) {}

/**
 * @brief Starts the communication.
 */
bool PortHandler::begin()
{
    // Initialize the Serial
    _serial.begin(TINY_SERIAL_BAUDRATE, SERIAL_8N1, TINY_SERIAL_RX_PIN, TINY_SERIAL_TX_PIN);
    // Clean the buffer
    unsigned long start = millis();
    while (millis() - start < 100)
        while (_serial.available())
            _serial.read();
    // Handshake (Try 3 times)
    for (int i = 0 ; i < 3 ; i++)
    {
        _sendPacket(TINY_CMD_PING, NULL, 0);
        uint8_t response;
        if (_readResponse(&response, 1) && response == 0xAA) return true;
        delay(50);
    }
    return false;
}

// --- GPIO CONTROL
/**
 * @brief Send command to configure a GPIO working mode.
 * @param pin GPIO number.
 * @param mode Working mode for the GPIO.
 */
void PortHandler::pinMode(uint8_t pin, uint8_t mode)
{
    uint8_t payload[] = { pin, mode };
    _sendPacket(TINY_CMD_PMODE, payload, 2);
    uint8_t ack; // Read the ACK but without blocking if fails
    _readResponse(&ack, 1);
}

/**
 * @brief Send command to change the state of a digital output.
 * @param pin GPIO number configured as digital output.
 * @param value State to set the GPIO (1 or 0, HIGH or LOW)
 */
void PortHandler::digitalWrite(uint8_t pin, uint8_t value)
{
    uint8_t payload[] = { pin, value };
    _sendPacket(TINY_CMD_DWRITE, payload, 2);
    uint8_t ack; // Read the ACK but without blocking if fails
    _readResponse(&ack, 1);
}

/**
 * @brief Send command to change a PWM value generated in an analog output.
 * @param pin GPIO number configured as analog output.
 * @param value Value to set the PWM (from 0 to 255).
 */
void PortHandler::analogWrite(uint8_t pin, uint8_t value)
{
    uint8_t payload[] = { pin, value };
    _sendPacket(TINY_CMD_AWRITE, payload, 2);
    uint8_t ack;
    _readResponse(&ack, 1);
}

/**
 * @brief Send command to read the actual state of a digital input.
 * @param pin GPIO number configured as digital input.
 * @return An integer (1 or 0, HIGH or LOW).
 */
int PortHandler::digitalRead(uint8_t pin)
{
    uint8_t payload[] = { pin };
    _sendPacket(TINY_CMD_DREAD, payload, 1);
    uint8_t value;
    if (_readResponse(&value, 1)) return value;
    return -1; // Error
}

/**
 * @brief Send command to read the actual analog value of an analog input.
 * @param pin GPIO number configured as analog input.
 * @return An integer from 0 to 4095 (0 to 3'3V).
 */
int PortHandler::analogRead(uint8_t pin)
{
    uint8_t payload[] = { pin };
    _sendPacket(TINY_CMD_AREAD, payload, 1);
    uint8_t buffer[2];
    delay(1);
    if (_readResponse(buffer, 2))
    {   
        int value = (uint16_t)( ((uint16_t)buffer[0] << 8) | (uint16_t)buffer[1] );
        return value;
    }
    return -1;
}

// --- I2C CONTROL
/**
 * @brief Send command to write in the I2C bus.
 * @param address Objetive device I2C address.
 * @param data Pointer to the data to be sent.
 * @param len Length of the data to be sent.
 * @return True if the transmission was successful.
 */
bool PortHandler::i2cWrite(uint8_t address, const uint8_t* data, size_t len)
{
    if (len > 32) return false; // Buffer limit crossed
    uint8_t payload[len + 1];
    payload[0] = address;
    memcpy(&payload[1], data, len);
    _sendPacket(TINY_CMD_I2C_W, payload, len + 1);
    uint8_t status;
    if (_readResponse(&status, 1)) return (status == 1);
    return false;
}

/**
 * @brief Send command to read from the I2C bus.
 * @param address Objetive device I2C address.
 * @param buffer Pointer to the buffer to store the data readen.
 * @param len Length of the data to be readen.
 * @return True if the transmission was successful.
 */
bool PortHandler::i2cRead(uint8_t address, uint8_t* buffer, size_t len)
{
    uint8_t payload[] = { address, (uint8_t)len };
    _sendPacket(TINY_CMD_I2C_R, payload, 2);
    return _readResponse(buffer, len);
}

// --- SPI CONTROL
/**
 * @brief Configure and start the SPI bus.
 * @param mode SPI working mode.
 * @param clock_divider SPI bus clock divider
 */
void PortHandler::spiBegin(uint8_t mode, uint8_t clock_divider)
{
    uint8_t payload[] = { mode, clock_divider };
    _sendPacket(TINY_CMD_SPI_I, payload, 2);
    uint8_t ack;
    _readResponse(&ack, 1);
}

/**
 * @brief Transfer data to the SPI bus.
 * @param buffer Pointer to the data to be sent.
 * @param len Length of the data to be sent.
 */
void PortHandler::spiTransfer(uint8_t* buffer, size_t len)
{
    const size_t MAX_CHUNK = 32;
    size_t processed = 0;
    while (processed < len)
    {
        size_t chunk = (len - processed > MAX_CHUNK) ? MAX_CHUNK : (len - processed);
        _sendPacket(TINY_CMD_SPI_T, &buffer[processed], chunk);
        if (!_readResponse(&buffer[processed], chunk)) break;
        processed += chunk;
    }
}

/**
 * @brief Sends a byte and received the response.
 * @param data Byte to be sent.
 * @return The response byte.
 * 
 */
uint8_t PortHandler::spiTransfer(uint8_t data)
{
    uint8_t byte = data;
    spiTransfer(&byte, 1);
    return byte;
}

/**
 * @brief Create in the ATtiny a watchdog over a GPIO.
 * @param pin GPIO where install the watchdog.
 * @param level Voltage level that must trigger the watchdog.
 */
void PortHandler::triggerRemoteSleep(uint8_t pin, uint8_t level)
{
    uint8_t payload[] = { pin, level };
    _sendPacket(TINY_CMD_SLEEP_MONITOR, payload, 2);
    uint8_t ack;
    _readResponse(&ack, 1);
}


// --- HELPERS
// Helper function to send a payload to the ATtiny.
void PortHandler::_sendPacket(uint8_t cmd, const uint8_t* payload, size_t len)
{
    uint8_t crc = _calcCRC(cmd, payload, len);
    _serial.write(TINY_START);
    _serial.write(cmd);
    _serial.write((uint8_t)len);
    if (len > 0) _serial.write(payload, len);
    _serial.write(crc);
}

// Helper function to read a response from the ATtiny.
bool PortHandler::_readResponse(uint8_t* buffer, size_t expected_len) // Response procotol is: [START] [LEN] [DATA...] [CRC]
{
    unsigned long start = millis();
    // Search START byte
    while (millis() - start < _timeout)
        if (_serial.available())
            if (_serial.read() == TINY_START) break;
    // Read the length
    while (!_serial.available() && (millis() - start < _timeout));
    if (!_serial.available()) return false;
    uint8_t rx_len = _serial.read();
    // Validate the length
    if (rx_len != expected_len)
    {
        while (_serial.available()) _serial.read();
        return false;
    }
    // Read the data and calculate CRC
    size_t count = 0;
    uint8_t crc_calculated = rx_len;
    while (count < rx_len && (millis() - start < _timeout))
    {
        if(_serial.available())
        {
            uint8_t byte = _serial.read();
            buffer[count++] = byte;
            crc_calculated ^= byte;
        }
    }
    if (count != rx_len) return false;
    // Read CRC
    while (!_serial.available() && (millis() - start < _timeout));
    if (!_serial.available()) return false; // Last byte missing
    uint8_t crc_received = _serial.read();
    // Validate CRC
    if (crc_calculated != crc_received) return false; // Data ERR
    return true; // Data OK
}

// Helper function to calculate the CRC of a payload.
uint8_t PortHandler::_calcCRC(uint8_t cmd, const uint8_t* data, size_t len)
{
    uint8_t crc = cmd ^ len;
    if (data != NULL)
        for (size_t i = 0 ; i < len ; i++) crc ^= data[i];
    return crc;
}
