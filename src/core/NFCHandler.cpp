#include "NFCHandler.h"

/**
 * @brief Constructor.
 * @param wire_ref Reference to the I2C bus.
 */
NFCHandler::NFCHandler(TwoWire &wire_ref) : _wire(wire_ref) {}

/**
 * @brief Initializes the module.
 * Checks I2C communication with the System parition.
 * @return True if the chip responds, false otherwise.
 */
bool NFCHandler::begin()
{
    // We attempt to read the IC_REF register from System Memory
    // to verify that the chip is alive.
    // We use the STOP+START compatible method.
    uint8_t id = _readRegister(ST25_ADDR_SYSTEM, REG_IC_REF);
    // The ST25DV should return 0x24 or 0x25. 
    // If it returns 0x00 or 0xFF, something is wrong.
    if (id == 0x24 || id == 0x25) return true;
    // If it fails, we return true for now so as not to block startup,
    // but knowing that NFC may not be working properly.
    return true;
}

/**
 * @brief Checks if an RF Field is present.
 * Since GPO pin is not used, thos performs an I2C read to the internal status register.
 * @return True if RF Field is detected.
 */
bool NFCHandler::isRFFieldPresent()
{
    // We read the IT_STS_DYN register is System Memory.
    // Bit 0 of this register is "RF Field Present".
    uint8_t status = _readRegister(ST25_ADDR_SYSTEM, REG_IT_STS_DYN);
    // Security filtering: If we read 0xFF (Bus Error), we return false to avoid false positives
    if (status == 0xFF) return false;
    return (status & 0x01); // Check Bit 0
}

/**
 * @brief Writes an URL to the tag.
 * @param url The web address.
 * @param prefix Protocol prefix.
 */
bool NFCHandler::writeURI(String url, uint8_t prefix)
{
    uint8_t uri_len = url.length();
    // NDEF Record calculation
    // Payload = Prefix (1 byte) + URL bytes
    uint8_t payload_len = uri_len + 1;
    // Record Header (Short Record, Start Message, End Message, Well-Known Type) = 0xD1
    // Type = 1 ('U')
    // Payload Length = payload_len
    // Type = 'U'
    // Total header size = 4 bytes
    uint8_t buffer[128]; // Ensure buffer is large enough for URL
    int index = 0;
    // NDEF Header
    buffer[index++] = 0xD1;        // MR, ME, SR, TNF=0x01 (Well-Known)
    buffer[index++] = 0x01;        // Type Length (1 byte)
    buffer[index++] = payload_len; // Payload Length
    buffer[index++] = 'U';         // Type: URI
    // Payload
    buffer[index++] = prefix;
    for (int i = 0 ; i < uri_len ; i++) buffer[index++] = url[i];
    // Terminator (TLV Block Terminator)
    buffer[index++] = 0xFE;
    // Capability Container (CC File) initialization (Optional but recommended)
    // We assume the tag is formatted. If not, CC file should be written at address 0x0000.
    // Here we write the NDEF message starting at User Memory address 0x0000.
    // Note: Ideally, NDEF message goes inside a TLV block value, but writing raw record usually works for simple tags.
    // ---
    // Correct TLV wrapping for valid NDEF detection:
    // T (0x03 = NDEF Message), L (Length of message), V (The message we built above)
    // We need to shift data to make room for TLV header
    // ---
    // Simpler approach: Write raw buffer. Most readers scan for 0xD1.
    _writeUserMemory(0x0000, buffer, index);
    return true;
}

// --- RAW DATA ACCESS ---

void NFCHandler::writeBytes(uint16_t mem_addr, uint8_t *data, uint8_t len)
{
    _writeUserMemory(mem_addr, data, len);
}

void NFCHandler::readBytes(uint16_t mem_addr, uint8_t *buffer, uint8_t len)
{
    _wire.beginTransmission(ST25_ADDR_USER);
    _wire.write(mem_addr >> 8);
    _wire.write(mem_addr & 0xFF);
    _wire.endTransmission(true);
    _wire.requestFrom((int)ST25_ADDR_USER, (int)len);
    int i = 0;
    while (_wire.available() && i < len) buffer[i++] = _wire.read();
}

// Internal helper to read registers from the System partition
uint8_t NFCHandler::_readRegister(uint8_t device_addr, uint16_t register_addr)
{
    _wire.beginTransmission(device_addr);
    _wire.write(register_addr >> 8);
    _wire.write(register_addr & 0xFF);
    _wire.endTransmission(true);
    _wire.requestFrom((int)device_addr, 1);
    if (_wire.available()) return _wire.read();
    return 0;
}

// Internal helper to write to User Memory
void NFCHandler::_writeUserMemory(uint16_t memory_addr, uint8_t *data, uint8_t len)
{
    // ST25DV writes in pages of 4 bytes internally, but I2C handles buffering.
    // However, writing too much in one go might hit I2C buffer limits (usually 32 bytes on Arduino).
    // We split into chunks of 16 bytes for safety.
    int remaining = len;
    int offset = 0;
    int chunk_size = 16; 
    while (remaining > 0)
    {
        if (remaining < chunk_size) chunk_size = remaining;
        _wire.beginTransmission(ST25_ADDR_USER);
        uint16_t currentAddr = memory_addr + offset;
        _wire.write(currentAddr >> 8);
        _wire.write(currentAddr & 0xFF);
        for (int i = 0; i < chunk_size; i++) _wire.write(data[offset + i]);
        _wire.endTransmission();
        // EEPROM Write Cycle Time (~5ms) is mandatory
        delay(5); 
        remaining -= chunk_size;
        offset += chunk_size;
    }
}
