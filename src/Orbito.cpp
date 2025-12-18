/**
 * @file Orbito.cpp
 * @brief Master Library for the Orbito Robot.
 * @details This library implements the Facade Pattern to unify all hardware drivers
 * (Camera, Display, AI, NFC, etc.) into a single, easy-to-use API.
 * @author Digital Codesign
 * @version 1.0.0
 */

#include "Orbito.h"
#include <esp_sleep.h>
#include <FS.h>
#include <LittleFS.h>
#include <vector>

// Aux Structure for BLE updates
struct BLESensorBinding {
    String name;
    float* ptr;
    float last_value;
};

// Static list to store links (BLE)
static std::vector<BLESensorBinding> _ble_sensors;

// Static variables for animations
static unsigned long _last_blink_time = 0;
static int _next_blink_interval = 3000;
static bool _is_animating = false;
static int16_t _current_pupil_x = 0;
static int16_t _current_pupil_y = 0;
static OrbitoRobot::ActionModule::Emotion _current_emotion = OrbitoRobot::ActionModule::NEUTRAL;

// Main Objtect creation
OrbitoRobot Orbito;

/**
 * @brief Constructor.
 */
OrbitoRobot::OrbitoRobot() :
    // Bus initialization
    _spi_bus(HSPI),
    _uart_bus(0),
    _i2c_bus(0),
    // Driver initialization
    _ioDriver(_uart_bus),
    _flashDriver(&_spi_bus, PIN_FLASH_CS),
    _displayDriver(&_spi_bus),
    _nfcDriver(_i2c_bus),
    // Sub-module initialization
    System(), Vision(), Display(), Action(),
    Brain(), Storage(), Connect(), Remote()
{
    _initialized = false;
    _aiAdapter = nullptr;
}

/**
 * @brief Initializes ALL hardware modules in the correct order.
 * @return true if critical hardware (Flash, ATtiny) initialized correctly.
 */
bool OrbitoRobot::begin()
{
    // Comms busses initialization
    _spi_bus.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, -1);
    _i2c_bus.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    // UART inits in PortHandler::begin()
    // Starts base system (Power & ATtiny)
    if (!System.begin()) return false;
    // Start the Flash external memory
    _flashDriver.begin();
    // Starts the TFT Display
    _displayDriver.begin();
    // Starts the NFC
    _nfcDriver.begin();
    // Set flag and finish
    _initialized = true;
    return true;
}

/**
 * @brief Main system loop. Must be called inside the Arduino loop().
 * Handles WiFi reconnection, OTA, BLE events, and Eye Animations.
 */
void OrbitoRobot::update()
{
    if (!_initialized) return;
    // Maintain WiFi & OTA services
    Connect.checkUpdates();
    // Maintain BLE links
    for (auto &sensor : _ble_sensors)
    {
        if (sensor.ptr != nullptr)
        {
            float current_value = *(sensor.ptr);
            if (abs(current_value - sensor.last_value) > 0.01f)
            {
                _bleDriver.updateVariable(sensor.name, current_value);
                sensor.last_value = current_value;
            }
        }
    }
    // Maintain animations
    if (_is_animating)
    {
        unsigned long now = millis();
        if (now - _last_blink_time > _next_blink_interval)
        {
            Action.blink();
            _last_blink_time = now;
            _next_blink_interval = random(3000, 6000);
            if (_current_emotion == ActionModule::NEUTRAL)
            {
                int rx = random(-10, 11);
                int ry = random(-5, 6);
                Action.lookAt(rx, ry);
            }
        }
    }
}

 // =============================================================
// 1. SYSTEM MODULE (The Body & Hardware Base)
// =============================================================
// --- Energy Management ---

/**
 * @brief Initializes power lines, buses, and checks hardware health.
 * @return true if all critical hardware (Flash, ATtiny) responded correctly.
 */
bool OrbitoRobot::SystemModule::begin()
{
    // Starts the communicacions with the ATtiny.
    if (!Orbito._ioDriver.begin()) return false;
    // Config the internal peripheral GPIO
    pinMode(PIN_BUTTON, INPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    return true;
}

/**
 * @brief Puts the robot into Deep Sleep for a specific duration.
 * @param seconds Time to sleep in seconds.
 */
void OrbitoRobot::SystemModule::sleep(uint64_t seconds)
{
    // Turn Off all peripheral
    Orbito.Display.turnOff();
    Orbito.Vision.stopWebStream();
    // Config timer (us)
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
    // Go to sleep
    esp_deep_sleep_start();
}

/**
 * @brief Puts the robot into Deep Sleep until an external event occurs.
 * @param wakeup_pin (Optional) GPIO that triggers wake-up (e.g., NFC interrupt).
 * @param active_level Logic level to trigger (0=LOW, 1=HIGH).
 */
void OrbitoRobot::SystemModule::hibernate(int wakeup_pin, int active_level)
{
    // Turn Off all peripheral
    Orbito.Display.turnOff();
    Orbito.Vision.stopWebStream();
    // WiFi/BT turn down automatically in Deep Sleep
    // Config the wake up with the button
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON, 0);
    // Config the wake up with the ATtiny (Wake-on-UART)
    if (wakeup_pin != -1)
    {
        // Send the command to the ATtiny
        Orbito._ioDriver.triggerRemoteSleep(wakeup_pin, active_level);
        // Config the Rx pin from ESP32 to wake up if it goes LOW (UART Start-Bit)
        uint64_t mask = (1ULL << PIN_TINY_RX);
        esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_LOW);
    }
    // Go to sleep
    esp_deep_sleep_start();
}

/**
 * @brief Performs a full software reboot of the ESP32.
 */
void OrbitoRobot::SystemModule::restart()
{
    esp_restart();
}

// --- GPIO Expansion (via ATtiny Co-processor) ---

/**
 * @brief Configures a pin on the expansion port.
 * @param pin Pin number on the ATtiny.
 * @param mode INPUT, OUTPUT, or INPUT_PULLUP.
 */
void OrbitoRobot::SystemModule::pinMode(uint8_t pin, uint8_t mode)
{
    Orbito._ioDriver.pinMode(pin, mode);
}

/**
 * @brief Writes a digital value to an expansion pin.
 */
void OrbitoRobot::SystemModule::digitalWrite(uint8_t pin, bool value)
{
    Orbito._ioDriver.digitalWrite(pin, value ? 1 : 0);
}

/**
 * @brief Writes a PWM value (0-255) to an expansion pin.
 */
void OrbitoRobot::SystemModule::analogWrite(uint8_t pin, int value)
{
    Orbito._ioDriver.analogWrite(pin, (uint8_t)value);
}

/**
 * @brief Reads a digital value from an expansion pin.
 */
int OrbitoRobot::SystemModule::digitalRead(uint8_t pin)
{
    return Orbito._ioDriver.digitalRead(pin);
}

/**
 * @brief Reads an analog value (ADC) from an expansion pin.
 */
int OrbitoRobot::SystemModule::analogRead(uint8_t pin)
{
    return Orbito._ioDriver.analogRead(pin);
}

// --- External Devices Bridge ---

/**
 * @brief Writes data to an I2C device connected to the expansion port.
 */
bool OrbitoRobot::SystemModule::i2cWrite(uint8_t addr, uint8_t* data, size_t len)
{
    return Orbito._ioDriver.i2cWrite(addr, data, len);
}

/**
 * @brief Reads data from an I2C device connected to the expansion port.
 */
bool OrbitoRobot::SystemModule::i2cRead(uint8_t addr, uint8_t* buffer, size_t len)
{
    return Orbito._ioDriver.i2cRead(addr, buffer, len);
}

/**
 * @brief Initializes the SPI bus on the expansion port.
 */
void OrbitoRobot::SystemModule::spiInit(uint8_t mode, uint8_t clk_div)
{
    Orbito._ioDriver.spiBegin(mode, clk_div);
}

/**
 * @brief Transfers a buffer over SPI (Send & Receive).
 */
void OrbitoRobot::SystemModule::spiTransfer(uint8_t* buffer, size_t len)
{
    Orbito._ioDriver.spiTransfer(buffer, len);
}

/**
 * @brief Transfers a single byte over SPI.
 */
uint8_t OrbitoRobot::SystemModule::spiTransfer(uint8_t data)
{
    return Orbito._ioDriver.spiTransfer(data);
}

// --- Extras ---

/**
 * @brief Checks the status of the built-in user button (if available).
 */
bool OrbitoRobot::SystemModule::getButtonStatus()
{
    return (::digitalRead(PIN_BUTTON) == LOW);
}

/**
 * @brief Generates a tone on the buzzer (if connected).
 * @param frequency Frequency in Hz.
 * @param duration Duration in milliseconds.
 */
void OrbitoRobot::SystemModule::tone(uint16_t frequency, uint32_t duration)
{
    ::tone(PIN_BUZZER, frequency, duration);
}

// =============================================================
// 2. VISION MODULE (The Eyes)
// =============================================================
// --- Streaming ---

/**
 * @brief Starts the MJPEG Video Server.
 * @param port HTTP Port (Default: 81 to avoid conflict with UI).
 */
void OrbitoRobot::VisionModule::startWebStream()
{
    // Link the camera to the Web Server - IMPORTANT!
    Orbito._webDriver.enableCamera(Orbito._cameraDriver);
    // Start the Web Server
    // This boot UI in Port 80 and Streaming in Port 81
    Orbito._webDriver.begin();
}

/**
 * @brief Stops the video server and frees resources.
 */
void OrbitoRobot::VisionModule::stopWebStream()
{
    Orbito._webDriver.stop();
}

// --- Capture ---

/**
 * @brief Captures a single frame into RAM.
 * @note You MUST call release() after using the frame to prevent memory leaks.
 * @return Pointer to camera_fb_t struct.
 */
camera_fb_t* OrbitoRobot::VisionModule::snapshot()
{
    return Orbito._cameraDriver.getFrame();
}

/**
 * @brief Captures a frame and saves it directly to Flash memory as JPEG.
 * @param filename Path to save (e.g., "/photo1.jpg").
 */
bool OrbitoRobot::VisionModule::saveSnapshot(String filename)
{

}

/**
 * @brief Frees the memory allocated for a snapshot.
 */
void OrbitoRobot::VisionModule::release(camera_fb_t* fb)
{
    Orbito._cameraDriver.releaseFrame(fb);
}

// --- Hardware Adjustment ---

void OrbitoRobot::VisionModule::setResolution(framesize_t size)
{
    Orbito._cameraDriver.setResolution(size);
}

void OrbitoRobot::VisionModule::setQuality(int quality)
{
    Orbito._cameraDriver.setQuality(quality);
}

void OrbitoRobot::VisionModule::setEffect(int effect)
{
    Orbito._cameraDriver.setSpecialEffect((CameraHandler::Special_Effect) effect);
}

void OrbitoRobot::VisionModule::setBrightness(int level)
{
    Orbito._cameraDriver.setBrightness(level);
}

void OrbitoRobot::VisionModule::setFlip(bool vertical, bool horizontal)
{
    Orbito._cameraDriver.setVFlip(vertical);
    Orbito._cameraDriver.setHMirror(horizontal);
}

void OrbitoRobot::VisionModule::setNightMode(bool enable)
{
    // This mode requires to enable the exposure control to let a higher gain.
    Orbito._cameraDriver.setExposureControl(true);
    Orbito._cameraDriver.setGainCeiling(enable ? GAINCEILING_8X : GAINCEILING_2X);
}

// =============================================================
// 3. DISPLAY MODULE (The Face)
// =============================================================
// --- Graphics (Adafruit GFX Wrapper) ---

void OrbitoRobot::DisplayModule::fillScreen(uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.fillScreen(color);
    });
}

void OrbitoRobot::DisplayModule::drawPixel(int x, int y, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.drawPixel(x, y, color);
    });
}

void OrbitoRobot::DisplayModule::drawLine(int x0, int y0, int x1, int y1, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.drawLine(x0, y0, x1, y1, color);
    });
}

void OrbitoRobot::DisplayModule::drawRect(int x, int y, int w, int h, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.drawRect(x, y, w, h, color);
    });
}

void OrbitoRobot::DisplayModule::fillRect(int x, int y, int w, int h, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.fillRect(x, y, w, h, color);
    });
}

void OrbitoRobot::DisplayModule::drawCircle(int x, int y, int r, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.drawCircle(x, y, r, color);
    });
}

void OrbitoRobot::DisplayModule::fillCircle(int x, int y, int r, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.fillCircle(x, y, r, color);
    });
}

void OrbitoRobot::DisplayModule::drawRoundRect(int x, int y, int w, int h, int r, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.drawRoundRect(x, y, w, h, r, color);
    });
}

void OrbitoRobot::DisplayModule::fillRoundRect(int x, int y, int w, int h, int r, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.fillRoundRect(x, y, w, h, r, color);
    });
}

void OrbitoRobot::DisplayModule::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.drawTriangle(x0, y0, x1, y1, x2, y2, color);
    });
}

void OrbitoRobot::DisplayModule::fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    });
}

void OrbitoRobot::DisplayModule::print(String text)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.print(text);
    });
}

void OrbitoRobot::DisplayModule::setCursor(int x, int y)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.setCursor(x, y);
    });
}

void OrbitoRobot::DisplayModule::setTextColor(uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.setTextColor(color);
    });
}

// --- Multimedia ---

/**
 * @brief Draws a raw Camera Frame (RGB565) onto the screen.
 */
void OrbitoRobot::DisplayModule::drawSnapshot(camera_fb_t* fb)
{
    if (!fb) return;
    // We only can only print an image directly if it's in RGB565 or Grayscale
    // For JPEG we need a complex decoder, not implemented
    if (fb->format == PIXFORMAT_JPEG) return;
    // Print the Camera Frame given
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        // If it's Grayscale (1 byte per pixel), we need to convert or use gray bitmap
        if (fb->format == PIXFORMAT_GRAYSCALE)
        {
            int w = fb->width;
            int h = fb->height;
            // Hold a buffer for ONE line (w * 2 bytes per color)
            uint16_t* line_buffer = (uint16_t*)malloc(w * sizeof(uint16_t));
            if (!line_buffer) return;
            for (int y = 0 ; y < h ; y++)
            {
                for (int x = 0 ; x < w ; x++)
                {
                    // Read the brightness
                    uint8_t gray = fb->buf[y * w + x];
                    // Convert 8-bit to RGB565 (R:5, G:6, B:5)
                    // R = gray >> 3, G = gray >> 2, B = gray >> 3
                    uint16_t color = ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
                    line_buffer[x] = color;
                }
                // Draw the complete line
                tft.drawRGBBitmap(0, y, line_buffer, w, 1);
            }
            // Free buffer holded from memory
            free(line_buffer);
        } else {
            // Asume RGB565 (2 bytes per pixel)
            tft.drawRGBBitmap(0, 0, (uint16_t*)fb->buf, fb->width, fb->height);
        }
    });
}

/**
 * @brief Draws a raw RGB565 bitmap array.
 */
void OrbitoRobot::DisplayModule::drawBitmap(int x, int y, const uint8_t* bmp, int w, int h, uint16_t color)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.drawBitmap(x, y, bmp, w, h, color);
    });
}

/**
 * @brief Draws a pre-loaded icon/emoji by name.
 */
void OrbitoRobot::DisplayModule::drawEmoji(String emojiName, int x, int y)
{
    // TODO: Switch - case with Every Emoji available.
}

// --- Visual Console ---

/**
 * @brief Prints text to a scrolling terminal on the screen (Debug mode).
 */
void OrbitoRobot::DisplayModule::consoleLog(String text)
{

}

/**
 * @brief Sets the text size (1, 2, 3...).
 */
void OrbitoRobot::DisplayModule::setFont(int size)
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.setTextSize(size);
    });
}

// --- Hardware ---

void OrbitoRobot::DisplayModule::turnOn()
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.enableDisplay(true);
    });
}

void OrbitoRobot::DisplayModule::turnOff()
{
    Orbito._displayDriver.draw([=](Adafruit_ST7789 &tft) {
        tft.enableDisplay(false);
    });
}

// =============================================================
// 4. ACTION MODULE (Personality & Emotions)
// =============================================================
// --- Helper to render Faces ---
static void _drawArc(int16_t x, int16_t y, int16_t r, int16_t start_angle, int16_t end_angle, uint16_t color)
{
    // Draw a dot if radius is too small
    if (r < 2)
    {
        Orbito.Display.drawPixel(x, y, color);
        return;
    }
    // Dynamic resolution
    int step = 6;
    float deg2rad = 0.0174532925f;
    float rad = start_angle * deg2rad;
    // Calculate the starting line coordinates
    int16_t x1 = x + (int16_t)(cosf(rad) * r);
    int16_t y1 = y + (int16_t)(sinf(rad) * r);
    int16_t x2, y2;
    // Draw all the lines for every step
    for (int i = start_angle + step ; i <=end_angle ; i += step)
    {
        rad = i * deg2rad;
        x2 = x + (int16_t)(cosf(rad) * r);
        y2 = y + (int16_t)(sinf(rad) * r);
        Orbito.Display.drawLine(x1, y1, x2, y2, color);
        x1 = x2;
        y1 = y2;
    }
}

static void _drawThickArc(int16_t x, int16_t y, int16_t r, uint8_t thickness, int16_t start, int16_t end, uint16_t color)
{
    for (int i = 0 ; i < thickness ; i++) _drawArc(x, y, r - i, start, end, color);
}

static void _drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color)
{
    float deg2rad = 0.0174532925f;
    int16_t x1 = x0 + rx;
    int16_t y1 = y0;
    // Draw a poligon with many faces
    for (int i = 5 ; i <= 360 ; i += 5)
    {
        float rad = i * deg2rad;
        int16_t x2 = x0 + (int16_t)(cosf(rad) * rx);
        int16_t y2 = y0 + (int16_t)(sinf(rad) * ry);
        Orbito.Display.drawLine(x1, y1, x2, y2, color);
        x1 = x2;
        y1 = y2;
    }
}

static void _fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color)
{
    // Draw line by line from top to bottom
    for (int16_t y = -ry ; y <= ry ; y++)
    {
        int16_t width = (int16_t)(rx * sqrtf(1.0f - (float)(y * y) / (float)(ry * ry)));
        Orbito.Display.drawLine(x0 - width, y0 + y, x0 + width, y0 + y, color);
    }
}

static void _renderEye(OrbitoRobot::ActionModule::EyeParams p) {
    uint16_t COLOR_BG = 0xFFFF;
    uint16_t COLOR_FG = 0x0000;
    int16_t current_h = p.height * p.open_factor;
    if (current_h < 2) current_h = 2;
    Orbito.Display.fillRect( p.x - (p.width / 2) - 2, p.y - (p.height / 2) - 2, p.width + 4, p.height + 4, COLOR_BG);
    _fillEllipse(p.x, p.y, p.width / 2, current_h / 2, COLOR_FG);
    int16_t brow_radius = (p.width / 2) + (p.width / 4);
    int16_t brow_y = p.y - brow_radius * 2 + 10;
    if (p.has_eyebrown)
    {
        if (p.is_left_eye)
        {
            switch (p.eyebr_type)
            {
                case 1: Orbito.Display.fillCircle(p.x - (p.width / 2), brow_y + 20, brow_radius, COLOR_BG); break;
                case 2: Orbito.Display.fillTriangle(p.x + (p.width / 2), p.y, p.x - p.width, p.y - (p.height / 2), p.x + (p.width / 2), p.y - (p.height / 2), COLOR_BG);
            }
        } else {
            switch (p.eyebr_type)
            {
                case 1: Orbito.Display.fillCircle(p.x + (p.width / 2), brow_y + 20, brow_radius, COLOR_BG); break;
                case 2: Orbito.Display.fillTriangle(p.x - (p.width / 2), p.y, p.x + p.width, p.y - (p.height / 2), p.x - (p.width / 2), p.y - (p.height / 2), COLOR_BG);
            }
        }
    }
}

static void _redrawEyes(float override_open = -1.0)
{
    // Constant values for the designs
    int16_t EYE_Y    = 85;
    int16_t EYE_W    = 60;
    int16_t EYE_H    = 110;
    int16_t GAP      = 85;
    int16_t CENTER_X = 160;
    // Basic configuration
    OrbitoRobot::ActionModule::EyeParams left  = { (int16_t)(CENTER_X - GAP), EYE_Y, EYE_W, EYE_H, _current_pupil_x, _current_pupil_y, 20, 0.8, true,  0 };
    OrbitoRobot::ActionModule::EyeParams right = { (int16_t)(CENTER_X + GAP), EYE_Y, EYE_W, EYE_H, _current_pupil_x, _current_pupil_y, 20, 0.8, false, 0 };
    // Apply actual emotion
    switch (_current_emotion)
    {
        case OrbitoRobot::ActionModule::WORRY:
            left.open_factor = 0.8;
            left.eyebr_type = 1;
            left.has_eyebrown = true;
            left.is_left_eye = true;
            right.open_factor = 0.8;
            right.eyebr_type = 1;
            right.has_eyebrown = true;
            right.is_left_eye = false;
            break;
        case OrbitoRobot::ActionModule::ANGRY:
            left.open_factor = 0.8;
            left.eyebr_type = 2;
            left.has_eyebrown = true;
            left.is_left_eye = true;
            right.open_factor = 0.8;
            right.eyebr_type = 2;
            right.has_eyebrown = true;
            right.is_left_eye = false;
            break;
        case OrbitoRobot::ActionModule::HAPPY:
            break;
        case OrbitoRobot::ActionModule::NEUTRAL:
            break;
        case OrbitoRobot::ActionModule::SURPRISE:
            break;
        case OrbitoRobot::ActionModule::SLEEPY:
            break;
        case OrbitoRobot::ActionModule::SAD:
            break;
    }
    if (override_open >= 0) {
        left.open_factor = override_open;
        right.open_factor = override_open;
    }
    _renderEye(left);
    _renderEye(right);
}

static void _renderMouth(OrbitoRobot::ActionModule::MouthParams p)
{
    uint16_t COLOR_BG = 0xFFFF;
    uint16_t COLOR_FG = 0x0000;
    int16_t x0 = p.x - (p.width / 2);
    int16_t y0 = p.y - (p.height / 2);
    switch (p.shape)
    {
        case 0: // Worry
            _drawThickArc(p.x, p.y + 80, 100, 10, 225, 315, COLOR_FG);
            break;
        case 1: // ANGRY
            _drawThickArc(p.x, p.y + 80, 100, 10, 240, 300, COLOR_FG);
            break;
        case 2: // HAPPY
            _drawThickArc(p.x, p.y - 80, 100, 10, 60, 120, COLOR_FG);
            break;
        case 3: // NEUTRAL
            Orbito.Display.fillRoundRect(x0, y0, p.width, p.height, p.height / 2, COLOR_FG);
            break;
        case 4: // SURPRISE
            break;
        case 5: // SLEEPY
            break;
        case 6: // SAD
            break;
    }
}

// --- Expressivity ---

/**
 * @brief Sets the base facial expression on the screen. NEUTRAL, HAPPY, SAD, ANGRY, SURPRISED, SLEEPY, LOVE
 */
void OrbitoRobot::ActionModule::setExpression(Emotion e)
{
    _current_emotion = e;
    _current_pupil_x = 0;
    _current_pupil_y = 0;
    // Clean the display
    Orbito.Display.fillScreen(0xFFFF);
    // Draw the mouth
    int16_t MOUTH_Y  = 190;
    int16_t CENTER_X = 160;
    OrbitoRobot::ActionModule::MouthParams mouth = { CENTER_X, MOUTH_Y, 80, 10, 3 };
    // Apply actual emotion
    switch (e) {
        case WORRY:    mouth.shape = 0; break;
        case ANGRY:    mouth.shape = 1; break;
        case HAPPY:    mouth.shape = 2; break;
        case NEUTRAL:  mouth.shape = 3; break;
        case SURPRISE: mouth.shape = 4; break;
        case SLEEPY:   mouth.shape = 5; break;
        case SAD:      mouth.shape = 6; break;
    }
    _renderMouth(mouth);
    _redrawEyes();
}

/**
 * @brief Enables automatic eye blinking and random pupil movement.
 */
void OrbitoRobot::ActionModule::animateEyes(bool enable)
{
    _is_animating = enable;
    if (enable)
    {
        _last_blink_time = millis();
        _next_blink_interval = random(2000, 5000);
    }
}

/**
 * @brief Moves the eyes to look at a specific relative coordinate (0-100%).
 */
void OrbitoRobot::ActionModule::lookAt(int x, int y)
{
    _current_pupil_x = x;
    _current_pupil_y = y;
    _redrawEyes();
}

/**
 * @brief Forces an immediate blink animation.
 */
void OrbitoRobot::ActionModule::blink()
{
    _redrawEyes(0.1);
    delay(100);
    _redrawEyes(-1.0);
}

// --- Communication ---

/**
 * @brief Displays a comic-style speech bubble with text.
 */
void OrbitoRobot::ActionModule::say(String text)
{

}

// =============================================================
// 5. BRAIN MODULE (Artificial Intelligence)
// =============================================================
// --- Configuration ---

/**
 * @brief Loads the Edge Impulse Adapter.
 * @param ai_adapter Instance of OrbitoAI (defined in user sketch).
 */
void OrbitoRobot::BrainModule::load(AIInterface& ai_adapter)
{
    // Dependence inyection: Store the reference the OrbitoAI object created by the user in the sketch.
    Orbito._aiAdapter = &ai_adapter;
}

// --- Inference ---

/**
 * @brief Runs inference on a Camera Frame (Object Detection / Classification).
 */
AIResult OrbitoRobot::BrainModule::predict(camera_fb_t* image)
{
    // Check for brain
    if (!isLoaded()) return { "NO_MODEL", 0.0f, 0.0f, false };
    // Check image is valid
    if (!image) return { "NO_IMAGE", 0.0f, 0.0f, false };
    // Call the model prediction function
    return Orbito._aiAdapter->predict(image);
}

/**
 * @brief Runs inference on raw sensor data (Gestures / Audio).
 */
AIResult OrbitoRobot::BrainModule::predict(float* data, size_t len)
{
    // Check for brain
    if (!isLoaded()) return { "NO_MODEL", 0.0f, 0.0f, false };
    // Check image is valid
    if (!data || len == 0) return { "EMPTY_DATA", 0.0f, 0.0f, false };
    // Call the model prediction function
    return Orbito._aiAdapter->predict(data, len);
}

// --- Management ---

/**
 * @brief Sets the minimum confidence threshold to consider a detection valid.
 */
void OrbitoRobot::BrainModule::setThreshold(float confidence)
{
    if (isLoaded()) Orbito._aiAdapter->setThreshold(confidence);
}

/**
 * @brief Checks if a model has been loaded successfully.
 */
bool OrbitoRobot::BrainModule::isLoaded()
{
    return (Orbito._aiAdapter != nullptr);
}

// =============================================================
// 6. STORAGE MODULE (Flash Memory)
// =============================================================
// --- File Operations ---

bool OrbitoRobot::StorageModule::writeFile(String path, String content)
{
    File file = LittleFS.open(_cleanPath(path), FILE_WRITE);
    if (!file) return false;
    file.print(content);
    file.close();
    return true;
}

bool OrbitoRobot::StorageModule::appendFile(String path, String content)
{
    File file = LittleFS.open(_cleanPath(path), FILE_APPEND);
    if (!file) return false;
    file.print(content);
    file.close();
    return true;
}

String OrbitoRobot::StorageModule::readFile(String path)
{
    File file = LittleFS.open(_cleanPath(path), FILE_READ);
    if (!file) return "";
    String buffer = file.readString();
    file.close();
    return buffer;
}

bool OrbitoRobot::StorageModule::exists(String path)
{
    return LittleFS.exists(_cleanPath(path));
}

bool OrbitoRobot::StorageModule::remove(String path)
{
    return LittleFS.remove(_cleanPath(path));
}

// --- Management ---

void OrbitoRobot::StorageModule::format()
{
    LittleFS.format();
}

String OrbitoRobot::StorageModule::listDir()
{
    String output = "";
    // Open the root directory
    File root = LittleFS.open("/");
    // Check if it exists and is a valid directory
    if (!root || !root.isDirectory()) return output;
    // Open the first file
    File file = root.openNextFile();
    // Check if some file exists
    if (!file) return output;
    // Prepare output String and continue with the next file if it exists
    while (file)
    {
        // We need to add a line ending jump if it's not the first element
        if (output.length() > 0) output += "\n";
        // Format: "/file.extension (XXXXX bytes)"
        String file_name = String(file.name());
        if (!file_name.startsWith("/")) file_name = "/" + file_name;
        // Prepare the output directories list
        if (file.isDirectory()) output += "[DIR] " + file_name;
        else output += file_name + "(" + String(file.size()) + "bytes)";
        // Pass to the next file
        file = root.openNextFile();
    }
    return output;
}

int OrbitoRobot::StorageModule::getTotalSpace()
{
    return LittleFS.totalBytes();
}

int OrbitoRobot::StorageModule::getUsedSpace()
{
    return LittleFS.usedBytes();
}

String _cleanPath(String path)
{
    if (path.startsWith ("/")) return path;
    return "/" + path;
}

// =============================================================
// 7. CONNECTIVITY MODULE (WiFi & Cloud)
// =============================================================
// --- WiFi ---

void OrbitoRobot::ConnModule::connect(const char* ssid, const char* pass)
{
    // Activate the NTP with default UTC+1
    Orbito._wifiDriver.enableNTP(3600, 3600);
    // Activate the OTA Updates by default
    Orbito._wifiDriver.enableOTA("admin");
    // Starts the connection with "Orbito" as default hostname
    Orbito._wifiDriver.connect(ssid, pass, "Orbito");

}

void OrbitoRobot::ConnModule::createAP(const char* ssid)
{
    Orbito._wifiDriver.createAccessPoint("OrbitoHost", ssid, NULL);
    Orbito.Display.consoleLog("AP Creado: " + String(ssid));
}

String OrbitoRobot::ConnModule::getIP()
{
    return Orbito._wifiDriver.getIP();
}

int OrbitoRobot::ConnModule::getSignalQuality()
{
    return Orbito._wifiDriver.getSignalQuality();
}

// --- Utilities ---

String OrbitoRobot::ConnModule::getTime()
{
    return Orbito._wifiDriver.getTimeString();
}

// --- OTA (Over-The-Air Updates) ---

void OrbitoRobot::ConnModule::checkUpdates()
{
    // This function must be called in the loop() to handle:
    // - WiFi reconnection if needed.
    // - OTA services confirmations.
    Orbito._wifiDriver.update();
}

// --- Web Dashboard (Port 80) ---

/**
 * @brief Sends a status message to the connected web client.
 */
void OrbitoRobot::ConnModule::setWebStatus(String msg)
{
    Orbito._webDriver.setStatus(msg);
}

/**
 * @brief Injects the HTML/CSS/JS code for the web interface.
 */
void OrbitoRobot::ConnModule::setWebInterface(const char* html_content)
{
    Orbito._webDriver.setUserInterface(html_content);
}

/**
 * @brief Sets the callback to handle API commands from the web.
 * e.g., /cmd?id=dance&value=1
 */
void OrbitoRobot::ConnModule::onWebCommand(std::function<void(String id, int value)> callback)
{
    Orbito._webDriver.setCommandCallback(callback);
}

// =============================================================
// 8. REMOTE MODULE (External Control)
// =============================================================
// --- BLE (Mobile App Dashboard) ---

/**
 * @brief Initializes the BLE service and sets the robot name.
 */
void OrbitoRobot::RemoteModule::initDashboard(String robot_name)
{
    Orbito._bleDriver.init(robot_name);
    Orbito._bleDriver.begin();
}

/**
 * @brief Adds a toggle switch to the App Dashboard.
 */
void OrbitoRobot::RemoteModule::addSwitch(String name, std::function<void(bool)> cb)
{
    Orbito._bleDriver.addSwitch(name, false, cb);
}

/**
 * @brief Adds a sensor gauge to the App Dashboard.
 */
void OrbitoRobot::RemoteModule::addSensor(String name, float* value_ptr)
{
    Orbito._bleDriver.addVariable(name, 0.0f);
    if (value_ptr != nullptr) _ble_sensors.push_back({ name, value_ptr, *value_ptr });
}

/**
 * @brief Sends text to the App's serial terminal.
 */
void OrbitoRobot::RemoteModule::sendSerial(String text)
{
    Orbito._bleDriver.print(text);
}

/**
 * @brief Sets callback for receiving text from the App.
 */
void OrbitoRobot::RemoteModule::onSerialReceive(std::function<void(String)> cb)
{
    Orbito._bleDriver.onDataReceived(cb);
}

// --- NFC (Accessories) ---

/**
 * @brief Checks if an RF Field (Mobile/Reader) is present.
 */
bool OrbitoRobot::RemoteModule::isFieldPresent()
{
    return Orbito._nfcDriver.isRFFieldPresent();
}

/**
 * @brief Reads the Unique ID of the tag (For identifying accessories).
 */
String OrbitoRobot::RemoteModule::readTagUID()
{
    return Orbito._wifiDriver.getMacAddress();
}

/**
 * @brief Reads NDEF text content (For notes/data).
 */
String OrbitoRobot::RemoteModule::readTagText()
{
    // Read the User Memory looking for a TEXT register (Type 'T')
    uint8_t buffer[64]; // Read the first 64 bytes
    Orbito._nfcDriver.readBytes(0x0000, buffer, 64);
    // Basic parse NDEF
    // Byte 0: 0xD1 (Inicio)
    // Byte 3: 'T' (Text type) or 'U' (URI type)
    if (buffer[0] == 0xD1)
    {
        if (buffer[3] == 'T')
        {
            uint8_t payload_len = buffer[2];
            uint8_t lang_len = buffer[4];
            // The text starts in buffer[4 + 1 + lang_len]
            // Header usual size = 4 bytes (header) + 1 byte (status/length) + Language length
            int start_index = 4 + 1 + buffer[4];
            int text_len = payload_len - (1 + buffer[4]);
            // Prepare function output
            char output[text_len + 1];
            memcpy(output, &buffer[start_index], text_len);
            output[text_len] = '\0';
            return String(output);
        } else return "";
    }
}

/**
 * @brief Writes a URL to the tag (Opens on mobile when tapped).
 */
void OrbitoRobot::RemoteModule::writeUrl(String url)
{
    Orbito._nfcDriver.writeURI(url);
}

/**
 * @brief Writes text data to the tag.
 */
void OrbitoRobot::RemoteModule::writeText(String txt)
{
    // Simplified implementation for NDEF TEXT register (Type 'T')
    // Header: [D1] [01] [Len] 'T' [02] 'e' 'n' [Payload...]
    uint8_t lang_len = 2;
    uint8_t text_len = txt.length();
    uint8_t payload_len = lang_len + 1 + text_len; // 1 byte for status
    uint8_t header[] = {
        0xD1,        // MB, ME, SR, TNF=Well Known
        0x01,        // Type Length
        payload_len, // Payload Length
        'T',         // Type: Text
        0x02,        // Status byte: UTF-8, lang len = 2
        'e', 'n'     // Language: English
    };
    // Write Header
    Orbito._nfcDriver.writeBytes(0x0000, header, sizeof(header));
    // Write Text
    Orbito._nfcDriver.writeBytes(0x0000 + sizeof(header), (uint8_t*)txt.c_str(), text_len);
    // TLV Ending
    uint8_t term = 0xFE;
    Orbito._nfcDriver.writeBytes(0x0000 + sizeof(header) + text_len, &term, 1);
}
