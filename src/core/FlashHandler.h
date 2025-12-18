#ifndef FLASH_HANDLER_H
#define FLASH_HANDLER_H

#include <Arduino.h>
#include "./SPIHandler.h"

// --- W25Q16 COMMANDS ---
#define W25Q_CMD_WRITE_ENABLE    0x06
#define W25Q_CMD_WRITE_DISABLE   0x04
#define W25Q_CMD_READ_STATUS_1   0x05
#define W25Q_CMD_READ_DATA       0x03
#define W25Q_CMD_PAGE_PROGRAM    0x02
#define W25Q_CMD_SECTOR_ERASE_4K 0x20
#define W25Q_CMD_BLOCK_ERASE_32K 0x52
#define W25Q_CMD_BLOCK_ERASE_64K 0xD8
#define W25Q_CMD_CHIP_ERASE      0xC7
#define W25Q_CMD_JEDEC_ID        0x9F
#define W25Q_CMD_POWER_DOWN      0xB9
#define W25Q_CMD_RELEASE_PD      0xAB

// --- W25Q16 LAYOUT
#define W25Q_PAGE_SIZE 256

class FlashHandler : public SPIHandler {

    public:

        /**
         * @brief Cosntructor.
         * @param spi_bus Pointer to the SPIClass object.
         * @param cs_pin Chip Select pin.
         */
        FlashHandler(SPIClass* spi_bus, int cs_pin);

        /**
         * @brief Initializes the flash and wakes it up.
         */
        void begin() override;

        /**
         * @brief Reads the JEDEC Manufacturer and Device ID.
         * @return 24-bit ID (Manufacturer + Memory Type + Capacity).
         * For W25Q16, it should be usually 0xEF4015.
         */
        uint32_t getJEDECID();

        /**
         * @brief Reads data from a specific address.
         * @param addr Starting byte address.
         * @param buffer Pointer to the array where data will be stored.
         * @param len Number of bytes to read.
         */
        void read(uint32_t addr, uint8_t* buffer, size_t len);

        /**
         * @brief Writes data to the flash.
         * Handles Page Program boundaries automatically.
         * @param addr Starting byte address.
         * @param buffer Pointer to the data to write.
         * @param len Number of bytes to write.
         */
        void write(uint32_t addr, const uint8_t* buffer, size_t len);

        /**
         * @brief Erases a 4KB Sector. The smallest erasable unit.
         * Takes roughly 45ms.
         * @param addr Any address inside the sector to be erased.
         */
        void eraseSector(uint32_t addr);

        /**
         * @brief Erases the entire chip (This can take several seconds).
         */
        void eraseChip();

        /**
         * @brief Checks if the flash is currently busy (Writing or Erasing).
         * @return True if busy.
         */
        bool isBusy();

        /**
         * @brief Blocks execution until the flash operation is complete.
         * It efficiently yields to other tasks while waiting.
         */
        void waitForReady();

        /**
         * @brief Puts the device in Deep Power-Down mode.
         */
        void powerDown();

        /**
         * @brief Wakes up the device from Deep Power-Down.
         */
        void wakeUp();

    private:

        // Internal helper to enable writing latch (WEL bit)
        void _writeEnable();
        // Helper to send a 24-bit address
        void _sendAddress(uint32_t addr);

};

#endif
