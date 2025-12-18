#ifndef BLEHANDLER_H
#define BLEHANDLER_H

#include <Arduino.h>
// Native ESP32 BLE libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// Advanced management C++ libraries
#include <map>
#include <functional>


/**
 * @brief UUID Definitions
 * We use the Nordic UART Service (NUS) standard. This ensures compatibility
 * with most generic mobile apps like "Serial Bluetooth Terminal" or "nRF Connect".
 */
#define BLE_UUID_UART_SERVICE    "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_UUID_RX              "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_UUID_TX              "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/**
 * @brief Base UUID for Dynamic Dashboard Variables
 * The last 4 digits (bytes) will be incremented for each new variable added.
 */
#define BLE_UUID_VAR_SERVICE     "12345678-1234-1234-1234-123456780000"
#define BLE_UUID_VAR_BASE        "12345678-1234-1234-1234-12345678" // Last 4 digits change for each new variable

/**
 * @brief Handles connection status changes (Connect/Disconnect).
 */
class ConnectionCallbacks : public BLEServerCallbacks
{

    public:

        bool* connected_flag_pointer;
        void (*userOnConnect)();
        void (*userOnDisconnect)();

        ConnectionCallbacks(bool* flag, void (*onConn)(), void(*onDisc)());
        void onConnect(BLEServer* server_pointer) override;
        void onDisconnect(BLEServer* server_pointer) override;

};

/**
 * @brief Handles incoming text data for UART Mode.
 */
class UartCallbacks : public BLECharacteristicCallbacks
{

    public:

        std::function<void(String)> onDataCallback;

        UartCallbacks(std::function<void(String)> callback);
        void onWrite(BLECharacteristic* characteristic_pointer) override;

};

/**
 * @brief Handles incoming boolean toggles for Dashboard Mode (Switches).
 */
class SwitchCallbacks : public BLECharacteristicCallbacks
{

    public:

        std::function<void(bool)> onSwitchCallback;

        SwitchCallbacks(std::function<void(bool)> callback);
        void onWrite(BLECharacteristic* characteristic_pointer) override;

};

/**
 * @brief Main Class for Bluetooth LE management
 */
class BLEHandler : public Print
{

    public:

        BLEHandler();

        /**
         * @brief Initializes the BLE Server but DOES NOT start advertising yet.
         * Call this first in your setup().
         * @param device_name The name that will appear during scanning (default: ESP32-S3).
         */
        void init(String device_name = "ESP32-S3");

        /**
         * @brief Starts all services and begins Advertising.
         * Call this at the VERY END of your setup().
         */
        void begin();

        /**
         * @brief Checks if a client is currently connected.
         * @return true if connected, false otherwise.
         */
        bool isConnected();

        // --- UART / SERIAL MODE METHODS ---

        /**
         * @brief Sets the callback function to execute when text data receives via UART service.
         */
        void onDataReceived(std::function<void(String)> callback);

        /**
         * @brief Sets the callback for connection event.
         */
        void onConnect(void (*callback)());

        /**
         * @brief Sets the callback for disconnection event.
         */
        void onDisconnect(void (*callback)());

        // Override Print methods to allow use of .print(), .println(), .printf()
        size_t write(uint8_t character) override;
        size_t write(const uint8_t *buffer, size_t size) override;

        // --- DASHBOARD / VARIABLE MODE METHODS ---

        /**
         * @brief Creates a read-only variable (sensor) readable by the App.
         * @param name Internal identifier for the variable.
         * @param initial_value Starting value.
         */
        void addVariable(String name, float initial_value);

        /**
         * @brief Creates a read/write switch (toggle) controllable by the App.
         * @param name Internal identifier.
         * @param initial_state Starting state (true/false).
         * @param callback Function to execute when the App changes the switch state.
         */
        void addSwitch(String name, bool initial_state, std::function<void(bool)> callback);

        /**
         * @brief Updates the value of an existing variable and notifies the App.
         */
        void updateVariable(String name, float new_value);

        /**
         * @brief Updates the state of an existing switch (e.g. from physical button).
         */
        void updateSwitch(String name, bool new_state);

    private:

        // Pointers to Native BLE Objects
        BLEServer* _server_pointer = NULL;
        BLEService* _uart_service_pointer = NULL;
        BLEService* _variable_service_pointer = NULL;
        BLECharacteristic* _tx_characteristic_pointer = NULL;
        BLECharacteristic* _rx_characteristic_pointer = NULL;

        bool _device_connected = false;
        int _variables_counter = 1;

        // Map to store dynamic characteristics: Key = Name, Value = Pointer to Characteristic
        std::map<String, BLECharacteristic*> _dynamic_variables;

        // Internal storage for user callbacks
        void (*_callbackOnConnect)() = NULL;
        void (*_callbackOnDisconnect)() = NULL;
        std::function<void(String)> _callbackOnData = NULL;

        // Helper to create unique UUIDs for variables
        String _generateVariableUUID();

};

#endif 
