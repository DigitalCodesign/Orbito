/**
 * @file Orbito.h
 * @brief Master Library for the Orbito Robot.
 * @details This library implements the Facade Pattern to unify all hardware drivers
 * (Camera, Display, AI, NFC, etc.) into a single, easy-to-use API.
 * @author Digital Codesign
 * @version 1.0.0
 */

#ifndef ORBITO_LIB_H
#define ORBITO_LIB_H

#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Configuration & Pin Definitions
#include "./config/Pinout.h"

// Core Drivers (Low Level Implementation)
#include "./core/CameraHandler.h"
#include "./core/NFCHandler.h"
#include "./core/PortHandler.h"
#include "./core/DisplayHandler.h"
#include "./core/FlashHandler.h"
#include "./core/BLEHandler.h"
#include "./core/WiFiHandler.h"
#include "./core/WebServerHandler.h"

// AI Interface (Contract for Dependency Injection)
#include "./core/AIInterface.h"

class OrbitoRobot {

    public:

        // =============================================================
        // 1. SYSTEM MODULE (The Body & Hardware Base)
        // =============================================================
        /**
         * @brief Manages power, internal buses, and the ATtiny co-processor.
         */
        struct SystemModule
        {
            // --- Energy Management ---
            
            /**
             * @brief Initializes power lines, buses, and checks hardware health.
             * @return true if all critical hardware (Flash, ATtiny) responded correctly.
             */
            bool begin();

            /**
             * @brief Puts the robot into Deep Sleep for a specific duration.
             * @param seconds Time to sleep in seconds.
             */
            void sleep(uint64_t seconds);

            /**
             * @brief Puts the robot into Deep Sleep until an external event occurs.
             * @param wakeup_pin (Optional) GPIO that triggers wake-up (e.g., NFC interrupt).
             * @param active_level Logic level to trigger (0=LOW, 1=HIGH).
             */
            void hibernate(int wakeup_pin = -1, int active_level = -1);

            /**
             * @brief Performs a full software reboot of the ESP32.
             */
            void restart();

            // --- GPIO Expansion (via ATtiny Co-processor) ---

            /**
             * @brief Configures a pin on the expansion port.
             * @param pin Pin number on the ATtiny.
             * @param mode INPUT, OUTPUT, or INPUT_PULLUP.
             */
            void pinMode(uint8_t pin, uint8_t mode);

            /**
             * @brief Writes a digital value to an expansion pin.
             */
            void digitalWrite(uint8_t pin, bool value);

            /**
             * @brief Writes a PWM value (0-255) to an expansion pin.
             */
            void analogWrite(uint8_t pin, int value);

            /**
             * @brief Reads a digital value from an expansion pin.
             */
            int digitalRead(uint8_t pin);

            /**
             * @brief Reads an analog value (ADC) from an expansion pin.
             */
            int analogRead(uint8_t pin);

            // --- External Devices Bridge ---

            /**
             * @brief Writes data to an I2C device connected to the expansion port.
             */
            bool i2cWrite(uint8_t addr, uint8_t* data, size_t len);

            /**
             * @brief Reads data from an I2C device connected to the expansion port.
             */
            bool i2cRead(uint8_t addr, uint8_t* buffer, size_t len);

            /**
             * @brief Initializes the SPI bus on the expansion port.
             */
            void spiInit(uint8_t mode = 0, uint8_t clk_div = 4);

            /**
             * @brief Transfers a buffer over SPI (Send & Receive).
             */
            void spiTransfer(uint8_t* buffer, size_t len);

            /**
             * @brief Transfers a single byte over SPI.
             */
            uint8_t spiTransfer(uint8_t data);

            // --- Extras ---

            /**
             * @brief Checks the status of the built-in user button (if available).
             */
            bool getButtonStatus();

            /**
             * @brief Generates a tone on the buzzer (if connected).
             * @param frequency Frequency in Hz.
             * @param duration Duration in milliseconds.
             */
            void tone(uint16_t frequency, uint32_t duration);

        } System;

        // =============================================================
        // 2. VISION MODULE (The Eyes)
        // =============================================================
        /**
         * @brief Manages the Camera sensor and Video Streaming.
         */
        struct VisionModule
        {
            // --- Streaming ---

            /**
             * @brief Starts the MJPEG Video Server.
             * @param port HTTP Port (Default: 81 to avoid conflict with UI).
             */
            void startWebStream();

            /**
             * @brief Stops the video server and frees resources.
             */
            void stopWebStream();

            // --- Capture ---

            /**
             * @brief Captures a single frame into RAM.
             * @note You MUST call release() after using the frame to prevent memory leaks.
             * @return Pointer to camera_fb_t struct.
             */
            camera_fb_t* snapshot();

            /**
             * @brief Captures a frame and saves it directly to Flash memory as JPEG.
             * @param filename Path to save (e.g., "/photo1.jpg").
             */
            bool saveSnapshot(String filename);

            /**
             * @brief Frees the memory allocated for a snapshot.
             */
            void release(camera_fb_t* fb);

            // --- Hardware Adjustment ---

            void setMode(CameraHandler::Camera_Mode mode);
            void setResolution(framesize_t size);   // QVGA, VGA, SVGA, UXGA...
            void setQuality(int quality);           // JPEG Quality (0-63)
            void setEffect(int effect);             // Hardware FX (Sepia, Negative...)
            void setBrightness(int level);          // -2 to 2
            void setFlip(bool vertical, bool horizontal);
            void setNightMode(bool enable);         // High gain/exposure configuration
        } Vision;

        // =============================================================
        // 3. DISPLAY MODULE (The Face)
        // =============================================================
        /**
         * @brief Manages the TFT Screen, Graphics, and Multimedia.
         */
        struct DisplayModule
        {

            // --- Graphics (Adafruit GFX Wrapper) ---
            
            void fillScreen(uint16_t color);
            void drawPixel(int x, int y, uint16_t color);
            void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
            void drawRect(int x, int y, int w, int h, uint16_t color);
            void fillRect(int x, int y, int w, int h, uint16_t color);
            void drawCircle(int x, int y, int r, uint16_t color);
            void fillCircle(int x, int y, int r, uint16_t color);
            void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color);
            void fillRoundRect(int x, int y, int w, int h, int r, uint16_t color);
            void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
            void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
            void print(String text);
            void setCursor(int x, int y);
            void setTextColor(uint16_t color);

            // --- Multimedia ---

            /**
             * @brief Draws a raw Camera Frame (RGB565) onto the screen.
             */
            void drawSnapshot(camera_fb_t* fb);

            /**
             * @brief Draws a raw RGB565 bitmap array.
             */
            void drawBitmap(int x, int y, const uint8_t* bmp, int w, int h, uint16_t color);

            /**
             * @brief Draws a pre-loaded icon/emoji by name.
             */
            void drawEmoji(String emojiName, int x, int y);

            // --- Visual Console ---

            /**
             * @brief Prints text to a scrolling terminal on the screen (Debug mode).
             */
            void consoleLog(String text);

            /**
             * @brief Sets the text size (1, 2, 3...).
             */
            void setFont(int size);

            // --- Hardware ---

            void turnOn();
            void turnOff();

        } Display;

        // =============================================================
        // 4. ACTION MODULE (Personality & Emotions)
        // =============================================================
        /**
         * @brief High-level control for robot expressions and interactions.
         */
        struct ActionModule
        {
            enum Emotion { WORRY, ANGRY, HAPPY, NEUTRAL, SURPRISE, SLEEPY, SAD };
            struct EyeParams {
                int16_t x;            // Position X (Center)
                int16_t y;            // Position Y (Center)
                int16_t width;        // Eye total width
                int16_t height;       // Eye total height
                int16_t pupil_x;      // Position X pupil (-15 to 15)
                int16_t pupil_y;      // Position Y pupil (-15 to 15)
                int16_t margin;       // Size difference between Eye and Pupil
                float open_factor;    // 0.0 (Close) a 1.0 (Open)
                bool has_eyebrown;    // Flag to draw an eyebrown
                bool is_left_eye;     // Flag to draw left or right eyebrown
                uint8_t eyebr_type;   // ID of eyebrown type to draw
            };
            struct MouthParams {
                int16_t x;           // Position X (Center)
                int16_t y;           // Position Y (Center)
                int16_t width;       // Mouth total width
                int16_t height;      // Mouth line thickness
                int8_t shape;        // 0=Neutral, 1=Smile, 2=Sad, 3=Surprise
            };

            // --- Expressivity ---

            /**
             * @brief Sets the base facial expression on the screen.
             */
            void setExpression(Emotion e);

            /**
             * @brief Enables automatic eye blinking and random pupil movement.
             */
            void animateEyes(bool enable);

            /**
             * @brief Moves the eyes to look at a specific relative coordinate (0-100%).
             */
            void lookAt(int x, int y);

            /**
             * @brief Forces an immediate blink animation.
             */
            void blink();

            // --- Communication ---

            /**
             * @brief Displays a comic-style speech bubble with text.
             */
            void say(String text);

        } Action;

        // =============================================================
        // 5. BRAIN MODULE (Artificial Intelligence)
        // =============================================================
        /**
         * @brief Unified Inference Engine for Vision and Sensor Data.
         */
        struct BrainModule
        {
            // --- Configuration ---

            /**
             * @brief Loads the Edge Impulse Adapter.
             * @param ai_adapter Instance of OrbitoAI (defined in user sketch).
             */
            void load(AIInterface& ai_adapter);

            // --- Inference ---

            /**
             * @brief Runs inference on a Camera Frame (Object Detection / Classification).
             */
            AIResult predict(camera_fb_t* image);

            /**
             * @brief Runs inference on raw sensor data (Gestures / Audio).
             */
            AIResult predict(float* data, size_t len);

            // --- Management ---

            /**
             * @brief Sets the minimum confidence threshold to consider a detection valid.
             */
            void setThreshold(float confidence);

            /**
             * @brief Checks if a model has been loaded successfully.
             */
            bool isLoaded();

        } Brain;

        // =============================================================
        // 6. STORAGE MODULE (Flash Memory)
        // =============================================================
        /**
         * @brief File System Manager (LittleFS/FATFS).
         */
        struct StorageModule
        {
            // --- File Operations ---
            bool writeFile(String path, String content);
            bool appendFile(String path, String content); // Ideal for logs
            String readFile(String path);
            bool exists(String path);
            bool remove(String path);

            // --- Management ---
            void format();      // Wipes the entire memory
            String listDir();   // Prints file list to Serial
            int getTotalSpace();
            int getUsedSpace();

            // --- Helper ---
            static String _cleanPath(String path);

        } Storage;

        // =============================================================
        // 7. CONNECTIVITY MODULE (WiFi & Cloud)
        // =============================================================
        /**
         * @brief Manages WiFi connection, Time, and Web Interface.
         */
        struct ConnModule
        {
            // --- WiFi ---
            void connect(const char* ssid, const char* pass);
            void createAP(const char* ssid);
            String getIP();
            int getSignalQuality(); // Returns 0-100%

            // --- Utilities ---
            String getTime(); // Gets time via NTP

            // --- OTA (Over-The-Air Updates) ---
            void checkUpdates();

            // --- Web Dashboard (Port 80) ---
            
            /**
             * @brief Sends a status message to the connected web client.
             */
            void setWebStatus(String msg);

            /**
             * @brief Injects the HTML/CSS/JS code for the web interface.
             */
            void setWebInterface(const char* html_content);

            /**
             * @brief Sets the callback to handle API commands from the web.
             * e.g., /cmd?id=dance&value=1
             */
            void onWebCommand(std::function<void(String id, int value)> callback);

        } Connect;

        // =============================================================
        // 8. REMOTE MODULE (External Control)
        // =============================================================
        /**
         * @brief Manages Bluetooth (App) and NFC (Accessories).
         */
        struct RemoteModule
        {
            // --- BLE (Mobile App Dashboard) ---
            
            /**
             * @brief Initializes the BLE service and sets the robot name.
             */
            void initDashboard(String robot_name);

            /**
             * @brief Adds a toggle switch to the App Dashboard.
             */
            void addSwitch(String name, std::function<void(bool)> cb);

            /**
             * @brief Adds a sensor gauge to the App Dashboard.
             */
            void addSensor(String name, float* value_ptr);

            /**
             * @brief Sends text to the App's serial terminal.
             */
            void sendSerial(String text);

            /**
             * @brief Sets callback for receiving text from the App.
             */
            void onSerialReceive(std::function<void(String)> cb);

            // --- NFC (Accessories) ---

            /**
             * @brief Checks if an RF Field (Mobile/Reader) is present.
             */
            bool isFieldPresent();

            /**
             * @brief Reads the Unique ID of the tag (For identifying accessories).
             */
            String readTagUID();

            /**
             * @brief Reads NDEF text content (For notes/data).
             */
            String readTagText();

            /**
             * @brief Writes a URL to the tag (Opens on mobile when tapped).
             */
            void writeUrl(String url);

            /**
             * @brief Writes text data to the tag.
             */
            void writeText(String txt);

        } Remote;

        // =============================================================
        // MAIN API
        // =============================================================

        /**
         * @brief Constructor.
         */
        OrbitoRobot();

        /**
         * @brief Initializes ALL hardware modules in the correct order.
         * @return true if critical hardware (Flash, ATtiny) initialized correctly.
         */
        bool begin();

        /**
         * @brief Main system loop. Must be called inside the Arduino loop().
         * Handles WiFi reconnection, OTA, BLE events, and Eye Animations.
         */
        void update();

    private:

        // --- REAL BUS INSTANCES ---
        SPIClass         _spi_bus;
        HardwareSerial   _uart_bus;
        TwoWire          _i2c_bus;

        // --- DRIVER INSTANCES (Hidden from User) ---
        AIInterface*     _aiAdapter;
        CameraHandler    _cameraDriver;
        DisplayHandler   _displayDriver;
        WiFiHandler      _wifiDriver;
        BLEHandler       _bleDriver;
        NFCHandler       _nfcDriver;
        WebServerHandler _webDriver;
        PortHandler      _ioDriver;
        FlashHandler     _flashDriver;

        // --- INTERNAL STATE ---
        bool _initialized;

        // --- FRIENDSHIPS ---
        // Granting modules access to private drivers
        friend struct SystemModule;
        friend struct VisionModule;
        friend struct DisplayModule;
        friend struct ActionModule;
        friend struct BrainModule;
        friend struct StorageModule;
        friend struct ConnModule;
        friend struct RemoteModule;

};

// Global Instance for Plug & Play usage
extern OrbitoRobot Orbito;

#endif
