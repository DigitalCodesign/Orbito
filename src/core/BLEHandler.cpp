#include "BLEHandler.h"

// --- Connection Callbacks ---

ConnectionCallbacks::ConnectionCallbacks(bool *flag, void (*onConn)(), void (*onDisc)())
{
    connected_flag_pointer = flag;
    userOnConnect = onConn;
    userOnDisconnect = onDisc;
}

void ConnectionCallbacks::onConnect(BLEServer *server_pointer)
{
    *connected_flag_pointer = true;
    if (userOnConnect) userOnConnect();
}

void ConnectionCallbacks::onDisconnect(BLEServer *server_pointer)
{
    *connected_flag_pointer = false;
    server_pointer->getAdvertising()->start();
    if (userOnDisconnect) userOnDisconnect();
}

// --- UART Callbacks ---

UartCallbacks::UartCallbacks(std::function<void(String)> callback) : onDataCallback(callback) {}

void UartCallbacks::onWrite(BLECharacteristic *characteristic_pointer)
{
    String rx_value = characteristic_pointer->getValue();
    if (rx_value.length() > 0 && onDataCallback) onDataCallback(String(rx_value.c_str()));
}

// --- Switch Callbacks ---

SwitchCallbacks::SwitchCallbacks(std::function<void(bool)> callback) : onSwitchCallback(callback) {}

void SwitchCallbacks::onWrite(BLECharacteristic *characteristic_pointer)
{
    String value = characteristic_pointer->getValue();
    if (value.length() > 0 && onSwitchCallback)
    {
        // '1' or 0x01 is considered TRUE
        bool state = (value[0] == '1' || value[0] == 1);
        onSwitchCallback(state);
    }
}

// --- Main Class implementation ---

BLEHandler::BLEHandler() {}

/**
 * @brief Initializes the BLE Stack and Server ONLY.
 * Does NOT start advertising yet.
 */
void BLEHandler::init(String device_name)
{
    BLEDevice::init(device_name.c_str());
    // Creates the server
    _server_pointer = BLEDevice::createServer();
    _server_pointer->setCallbacks(new ConnectionCallbacks(
        &_device_connected, _callbackOnConnect, _callbackOnDisconnect
    ));
}

/**
 * @brief Initializes the BLE Stack, Server, Services, and Advertising.
*/
void BLEHandler::begin()
{
    // Creates the UART service
    _uart_service_pointer = _server_pointer->createService(BLE_UUID_UART_SERVICE);
    // Creates TX Characteristic
    _tx_characteristic_pointer = _uart_service_pointer->createCharacteristic(
        BLE_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY
    );
    //_tx_characteristic_pointer->addDescriptor(new BLE2902());
    // Creates RX Characteristic
    _rx_characteristic_pointer = _uart_service_pointer->createCharacteristic(
        BLE_UUID_RX, BLECharacteristic::PROPERTY_WRITE
    );
    // If the user configured onDataReceived before begin
    _rx_characteristic_pointer->setCallbacks(new UartCallbacks(_callbackOnData));
    _uart_service_pointer->start();
    // Start variable service
    if (_variable_service_pointer) _variable_service_pointer->start();
    // Start publishing
    BLEAdvertising* advertising_pointer = BLEDevice::getAdvertising();
    advertising_pointer->addServiceUUID(BLE_UUID_UART_SERVICE);
    advertising_pointer->setScanResponse(true);
    advertising_pointer->setMinPreferred(0x06);
    advertising_pointer->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

/**
 * @brief Checks if a client is currently connected.
 * @return true if connected, false otherwise.
 */
bool BLEHandler::isConnected()
{
    return _device_connected;
}

/**
 * @brief Sets the callback function to execute when text data receives via UART service.
 */
void BLEHandler::onDataReceived(std::function<void(String)> callback)
{
    _callbackOnData = callback;
    // If we started already, we update the callback of RX
    if (_rx_characteristic_pointer) _rx_characteristic_pointer->setCallbacks(new UartCallbacks(_callbackOnData));
}

/**
 * @brief Sets the callback for connection event.
 */
void BLEHandler::onConnect(void (*callback)())
{
    _callbackOnConnect = callback;
}

/**
 * @brief Sets the callback for disconnection event.
 */
void BLEHandler::onDisconnect(void (*callback)())
{
    _callbackOnDisconnect = callback;
}

size_t BLEHandler::write(uint8_t character)
{
    return write(&character, 1);
}

size_t BLEHandler::write(const uint8_t *buffer, size_t size)
{
    if (_device_connected && _tx_characteristic_pointer)
    {
        _tx_characteristic_pointer->setValue((uint8_t*)buffer, size);
        _tx_characteristic_pointer->notify();
        delay(5);
        return size;
    }
    return 0;
}

/**
 * @brief Creates a read-only variable (sensor) readable by the App.
 * @param name Internal identifier for the variable.
 * @param initial_value Starting value.
 */
void BLEHandler::addVariable(String name, float initial_value)
{
    if (!_variable_service_pointer) _variable_service_pointer = _server_pointer->createService(BLE_UUID_VAR_SERVICE);
    String uuid = _generateVariableUUID();
    BLECharacteristic* char_pointer = _variable_service_pointer->createCharacteristic(
        uuid.c_str(), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    //char_pointer->addDescriptor(new BLE2902());
    String value_string = String(initial_value, 2);
    char_pointer->setValue((uint8_t*)value_string.c_str(), value_string.length());
    _dynamic_variables[name] = char_pointer;
}

/**
 * @brief Creates a read/write switch (toggle) controllable by the App.
 * @param name Internal identifier.
 * @param initial_state Starting state (true/false).
 * @param callback Function to execute when the App changes the switch state.
 */
void BLEHandler::addSwitch(String name, bool initial_state, std::function<void(bool)> callback)
{
    if (!_variable_service_pointer) _variable_service_pointer = _server_pointer->createService(BLE_UUID_VAR_SERVICE);
    String uuid = _generateVariableUUID();
    BLECharacteristic* char_pointer = _variable_service_pointer->createCharacteristic(
        uuid.c_str(), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
    );
    //char_pointer->addDescriptor(new BLE2902());
    char_pointer->setCallbacks(new SwitchCallbacks(callback));
    String value_string = initial_state ? "1" : "0";
    char_pointer->setValue((uint8_t*)value_string.c_str(), value_string.length());
    _dynamic_variables[name] = char_pointer;
}

/**
 * @brief Updates the value of an existing variable and notifies the App.
 */
void BLEHandler::updateVariable(String name, float new_value)
{
    if(_device_connected && _dynamic_variables.count(name))
    {
        BLECharacteristic* char_pointer = _dynamic_variables[name];
        String value_string = String(new_value, 2);
        char_pointer->setValue((uint8_t*)value_string.c_str(), value_string.length());
        char_pointer->notify();
    }
}

/**
 * @brief Updates the state of an existing switch (e.g. from physical button).
 */
void BLEHandler::updateSwitch(String name, bool new_state)
{
    if(_device_connected && _dynamic_variables.count(name))
    {
        BLECharacteristic* char_pointer = _dynamic_variables[name];
        String value_string = new_state ? "1" : "0";
        char_pointer->setValue((uint8_t*)value_string.c_str(), value_string.length());
        char_pointer->notify();
    }
}

// Helper to create unique UUIDs for variables
String BLEHandler::_generateVariableUUID()
{
    char buffer[37];
    sprintf(buffer, "%s%04x", BLE_UUID_VAR_BASE, _variables_counter++);
    return String(buffer);
}
