#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>    // <-- Required for Serial prints, can be removed once debugged
#include <WiFi.h>       // <-- Required for connectivity functionalities
#include <ESPmDNS.h>    // <-- Required to map hostname to IP address
#include <ArduinoOTA.h> // <-- Required for OTA updates service
#include <time.h>       // <-- Required for NTP hour service

class WiFiHandler {

    public:

        // Constructor
        WiFiHandler(void);

        // Connect to a WiFi network
        void connect(const char* ssid, const char* password, const char* hostname = "ESP32Device");
        // Create a WiFi Access Point
        void createAccessPoint(const char* hostname, const char* ssid, const char* password = NULL);
        // Stablish a static IP
        void setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet);

        // Update WiFi status (call always in loop function)
        void update();

        // Starts the OTA updates service
        void enableOTA(const char* password_ota = "admin");

        // Starts the NTP hour service
        void enableNTP(long gmt_offset_sec = 3600, int daylight_offset_sec = 3600);
        // Get the actual time
        String getTimeString();

        // Energy management (true = Low power mode, false = Max power mode)
        void setPowerSavingMode(bool enable);

        // Check if connected to WiFi
        bool isConnected();
        // Get the Received Signal Strength Indicator (RSSI)
        int getRSSI();
        // Get the signal quality in %
        int getSignalQuality();
        // Get the local IP address
        String getIP();
        // Get the MAC Address
        String getMacAddress();

        // Only for AP mode: Get the number of devices connected
        int getAPClientCount();

    private:

        // Credentials and settings
        const char* _ssid;
        const char* _password;
        const char* _hostname;

        // Internal states
        bool _is_AP_mode;
        bool _mdns_started;
        bool _ota_enabled;
        bool _ntp_enabled;

        // Static IP configuration
        bool _use_static_ip;
        IPAddress _local_ip;
        IPAddress _gateway;
        IPAddress _subnet;

        // Variables for reconnection logic
        unsigned long _last_reconnect_attempt;
        unsigned long _reconnect_interval;

        // Start mDNS service
        void _startMDNS();
        // Configure OTA service
        void _setupOTA(const char* password);

};

#endif
