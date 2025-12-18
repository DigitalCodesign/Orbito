#include "WiFiHandler.h"

// Constructor
WiFiHandler::WiFiHandler(void)
{
    _is_AP_mode = false;
    _mdns_started = false;
    _ota_enabled = false;
    _ntp_enabled = false;
    _last_reconnect_attempt = 0;
    _reconnect_interval = 5000;
    _use_static_ip = false;
}

// Connect to a WiFi network
void WiFiHandler::connect(const char* ssid, const char* password, const char* hostname)
{
    // Setup credentials for client
    _ssid = ssid;
    _password = password;
    _hostname = hostname;
    _is_AP_mode = false;
    // Initial configuration
    WiFi.mode(WIFI_STA);
    // Configure static IP if needed
    if (_use_static_ip) WiFi.config(_local_ip, _gateway, _subnet);
    // Important to identify the robot in the Network
    WiFi.setHostname(_hostname);
    // Start the connection, but NOT wait for the CONNECTED state
    WiFi.begin(_ssid, _password);
    // By default disable the low power mode
    WiFi.setSleep(false);
}

// Create a WiFi Access Point
void WiFiHandler::createAccessPoint(const char* hostname, const char* ssid, const char* password)
{
    // Setup crdentials for AP
    _ssid = ssid;
    _password = password;
    _is_AP_mode = true;
    _hostname = hostname;
    // Initial configuration
    WiFi.mode(WIFI_AP);
    // Start the AP, if the password is NULL the network will have free access
    WiFi.softAP(_ssid, _password);
}

// Stablish a static IP
void WiFiHandler::setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet)
{
    _use_static_ip = true;
    _local_ip = ip;
    _gateway = gateway;
    _subnet = subnet;
}

// Starts the OTA updates service
void WiFiHandler::enableOTA(const char* password_ota)
{
    _setupOTA(password_ota);
    _ota_enabled = true;
}

// Starts the NTP hour service
void WiFiHandler::enableNTP(long gmt_offset_sec, int daylight_offset_sec)
{
    configTime(gmt_offset_sec, daylight_offset_sec, "pool.ntp.org", "time.nist.gov");
    _ntp_enabled = true;
}

// Get the actual time
String WiFiHandler::getTimeString()
{
    if (!_ntp_enabled) return "NTP Disabled";
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "Time Error";
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
    return String(timeStringBuff);
}

// Energy management (true = Low power mode, false = Max power mode)
void WiFiHandler::setPowerSavingMode(bool enable)
{
    WiFi.setSleep(enable);
}

// Update WiFi status
void WiFiHandler::update()
{
    // If OTA is enable, handle the service
    if (_ota_enabled) ArduinoOTA.handle();
    // If AP mode is enable, this method is not needed
    if (_is_AP_mode) return;
    // Check if we are connected to an AP
    if (WiFi.status() != WL_CONNECTED)
    {
        // Restart flag for mDNS
        _mdns_started = false;
        // Check if the waiting time has passed
        unsigned long current_millis = millis();
        if (current_millis - _last_reconnect_attempt >= _reconnect_interval)
        {
            _last_reconnect_attempt = current_millis;
            WiFi.reconnect();
        }
    } else {
        if (!_mdns_started) _startMDNS();
    }
}

// Start mDNS service
void WiFiHandler::_startMDNS()
{
    if (MDNS.begin(_hostname)) _mdns_started = true;
}

// Check if connected to WiFi
bool WiFiHandler::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

// Get the Received Signal Strength Indicator (RSSI)
int WiFiHandler::getRSSI()
{
    if (_is_AP_mode) return 0;
    return WiFi.RSSI();
}

// Get the signal quality in %
int WiFiHandler::getSignalQuality()
{
    if (_is_AP_mode) return 100;
    int rssi = getRSSI();
    // Map the value: -100 bDm is 0%, -50 dBm is 100%
    int quality = 0;
    if (rssi <= -100) quality = 0;
    else if (rssi >= -50) quality = 100;
    else quality = 2 * (rssi + 100);
    return quality;
}

// Get the local IP address
String WiFiHandler::getIP()
{
    if (_is_AP_mode) return WiFi.softAPIP().toString();
    if (isConnected()) return WiFi.localIP().toString();
    return "0.0.0.0";
}

// Get the MAC Address
String WiFiHandler::getMacAddress()
{
    return WiFi.macAddress();
}

// Only for AP mode: Get the number of devices connected
int WiFiHandler::getAPClientCount()
{
    if (!_is_AP_mode) return 0;
    return WiFi.softAPgetStationNum();
}

// Configure OTA service
void WiFiHandler::_setupOTA(const char* password)
{
    // Basic configuration of OTA service
    ArduinoOTA.setHostname(_hostname);
    if (password != NULL) ArduinoOTA.setPassword(password);
    // Callbacks configuration for each situation (onStart, onEnd, on Progress, onError)
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
        else type = "filesystem";
    });
    // Start the service
    ArduinoOTA.begin();
}
