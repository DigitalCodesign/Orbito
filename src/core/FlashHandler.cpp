#include "FlashHandler.h"

/**
 * @brief Cosntructor.
 * @param spi_bus Pointer to the SPIClass object.
 * @param cs_pin Chip Select pin.
 */
FlashHandler::FlashHandler(SPIClass* spi_bus, int cs_pin)
    : SPIHandler(spi_bus, cs_pin, 40000000) {}

/**
 * @brief Initializes the flash and wakes it up.
 */
void FlashHandler::begin()
{
    SPIHandler::begin(); // Init CS pin and Mutex
    wakeUp();            // Ensure flash is not in power down mode
    delay(5);            // Startup time
}

/**
 * @brief Reads the JEDEC Manufacturer and Device ID.
 * @return 24-bit ID (Manufacturer + Memory Type + Capacity).
 * For W25Q16, it should be usually 0xEF4015.
 */
uint32_t FlashHandler::getJEDECID()
{
    uint32_t id = 0;
    startTransaction();
    spiWrite(W25Q_CMD_JEDEC_ID);
    id |= (uint32_t)spiRead() << 16; // Manufacturer
    id |= (uint32_t)spiRead() << 8;  // Memory Type
    id |= (uint32_t)spiRead();       // Capacity
    endTransaction();
    return id;
}

/**
 * @brief Reads data from a specific address.
 * @param addr Starting byte address.
 * @param buffer Pointer to the array where data will be stored.
 * @param len Number of bytes to read.
 */
void FlashHandler::read(uint32_t addr, uint8_t* buffer, size_t len)
{
    waitForReady(); // Ensure no previous write is pending
    startTransaction();
    spiWrite(W25Q_CMD_READ_DATA);
    _sendAddress(addr);
    for (size_t i = 0 ; i < len ; i++) // Read bytes into buffer
        buffer[i] = spiRead();
    endTransaction();
}

/**
 * @brief Writes data to the flash.
 * Handles Page Program boundaries automatically.
 * @param addr Starting byte address.
 * @param buffer Pointer to the data to write.
 * @param len Number of bytes to write.
 */
void FlashHandler::write(uint32_t addr, const uint8_t* buffer, size_t len)
{
    uint32_t current_addr = addr;
    size_t remaining_bytes = len;
    size_t data_offset = 0;
    while (remaining_bytes > 0)
    {
        waitForReady(); // Wait for previous page to finish
        _writeEnable(); // Enable write latch
        startTransaction();
        spiWrite(W25Q_CMD_PAGE_PROGRAM);
        _sendAddress(current_addr);
        // Calculate how many bytes we can write in the current page
        // A page is 256 bytes. We cannot cross the boundary 255 -> 0 in one command.
        uint16_t page_offset = current_addr % W25Q_PAGE_SIZE;
        uint16_t bytes_available_in_page = W25Q_PAGE_SIZE - page_offset;
        uint16_t bytes_to_write = (remaining_bytes < bytes_available_in_page) ? remaining_bytes : bytes_available_in_page;
        // Send data
        for (size_t i = 0 ; i < bytes_to_write ; i++)
            spiWrite(buffer[data_offset + i]);
        endTransaction();
        // Update counters
        current_addr += bytes_to_write;
        data_offset += bytes_to_write;
        remaining_bytes -= bytes_to_write;
    }
}

/**
 * @brief Erases a 4KB Sector. The smallest erasable unit.
 * Takes roughly 45ms.
 * @param addr Any address inside the sector to be erased.
 */
void FlashHandler::eraseSector(uint32_t addr)
{
    waitForReady(); // Wait for previous page to finish
    _writeEnable(); // Enable write latch
    startTransaction();
    spiWrite(W25Q_CMD_SECTOR_ERASE_4K);
    _sendAddress(addr);
    endTransaction();
    // We do NOT wait here inside the mutex lock.
    // This allows other SPI devices to use the bus while Flash erases internally.
}

/**
 * @brief Erases the entire chip (This can take several seconds).
 */
void FlashHandler::eraseChip()
{
    waitForReady(); // Wait for previous page to finish
    _writeEnable(); // Enable write latch
    startTransaction();
    spiWrite(W25Q_CMD_CHIP_ERASE);
    endTransaction();
    // We do NOT wait here inside the mutex lock.
    // This allows other SPI devices to use the bus while Flash erases internally.
}

/**
 * @brief Checks if the flash is currently busy (Writing or Erasing).
 * @return True if busy.
 */
bool FlashHandler::isBusy()
{
    uint8_t status = 0;
    startTransaction();
    spiWrite(W25Q_CMD_READ_STATUS_1);
    status = spiRead();
    endTransaction();
    // Check Bit 0 (WIP: Work In Progress)
    return (status & 0x01);
}

/**
 * @brief Blocks execution until the flash operation is complete.
 * It efficiently yields to other tasks while waiting.
 */
void FlashHandler::waitForReady()
{
    // Polling loop. We release the mutex lock in each iteration
    while (isBusy())
        vTaskDelay(1 / portTICK_PERIOD_MS);
}

/**
 * @brief Puts the device in Deep Power-Down mode.
 */
void FlashHandler::powerDown()
{
    startTransaction();
    spiWrite(W25Q_CMD_POWER_DOWN);
    endTransaction();
}

/**
 * @brief Wakes up the device from Deep Power-Down.
 */
void FlashHandler::wakeUp()
{
    startTransaction();
    spiWrite(W25Q_CMD_RELEASE_PD);
    endTransaction();
}

// Internal helper to enable writing latch (WEL bit)
void FlashHandler::_writeEnable()
{
    startTransaction();
    spiWrite(W25Q_CMD_WRITE_ENABLE);
    endTransaction();
}

// Helper to send a 24-bit address
void FlashHandler::_sendAddress(uint32_t addr)
{
    // W25Q16 uses 24-bit address (A23 - A0)
    spiWrite((addr >> 16) & 0xFF);
    spiWrite((addr >> 8) & 0xFF);
    spiWrite(addr & 0xFF);
}
