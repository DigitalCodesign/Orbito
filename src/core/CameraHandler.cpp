#include "CameraHandler.h"

// Constructor
CameraHandler::CameraHandler()
{
    _sensor = NULL;
    _current_mode = MODE_STREAMING;
    _is_initialized = false;
}

// Initialize the Camera
bool CameraHandler::init(Camera_Mode mode)
{
    // Store the working mode
    _current_mode = mode;
    // Create the camera config struct
    camera_config_t config;
    // Fill the camera config struct, first pinout configuration
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    // Basic camera hardware configuration
    config.xclk_freq_hz = 20000000;
    // Specific camera hardware configuration by mode
    _configureCameraByMode(config, mode);
    // Camera initialization
    esp_err_t result = esp_camera_init(&config);
    if (result != ESP_OK) return false;
    // Sensor initialization
    _sensor = esp_camera_sensor_get();
    _applySensorSettings();
    // Set initialization flag
    _is_initialized = true;
    return true;
}

// Get the current initialization status
bool CameraHandler::isInitialized()
{
    return _is_initialized;
}

// Get a frame (a picture)
camera_fb_t *CameraHandler::getFrame()
{
    if (!_is_initialized || _sensor == NULL) return nullptr;
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb == nullptr) return nullptr;
    return fb;
}

// Free the RAM from the last frame
void CameraHandler::releaseFrame(camera_fb_t *fb)
{
    if (fb == NULL) return;
    esp_camera_fb_return(fb);
}

// Configure the image resolution
void CameraHandler::setResolution(framesize_t size)
{
    if (_sensor->pixformat == PIXFORMAT_JPEG)
        _sensor->set_framesize(_sensor, size);
}

// Configure the image quality
void CameraHandler::setQuality(int quality)
{
    if(_sensor == NULL) return;
    _sensor->set_quality(_sensor, quality);
}

// Configure the Vertical Image Flip
void CameraHandler::setVFlip(bool enable)
{
    if(_sensor == NULL) return;
    _sensor->set_vflip(_sensor, enable);
}

// Configure the Horizontal Image Mirror
void CameraHandler::setHMirror(bool enable)
{
    if(_sensor == NULL) return;
    _sensor->set_hmirror(_sensor, enable);
}

// Configure the image Brightness
void CameraHandler::setBrightness(int level)
{
    if(_sensor == NULL) return;
    _sensor->set_brightness(_sensor, level);
}

// Configure the image saturation
void CameraHandler::setSaturation(int level)
{
    if(_sensor == NULL) return;
    _sensor->set_saturation(_sensor, level);
}

// Configure the image contrast
void CameraHandler::setContrast(int level)
{
    if(_sensor == NULL) return;
    _sensor->set_contrast(_sensor, level);
}

// Configure the image White Balance
void CameraHandler::setWhiteBalance(bool enable, int mode)
{
    if (_sensor == NULL) return;
    _sensor->set_whitebal(_sensor, enable ? 1 : 0);
    _sensor->set_awb_gain(_sensor, enable ? 1 : 0);
    if (enable) _sensor->set_wb_mode(_sensor, mode);
}

// Configure the image exposure control
void CameraHandler::setExposureControl(bool enable, int dsp_level)
{
    if (_sensor == NULL) return;
    _sensor->set_exposure_ctrl(_sensor, enable ? 1 : 0);
    _sensor->set_aec2(_sensor, enable ? 1 : 0);
    if (enable && dsp_level >= -2 && dsp_level <= 2)
        _sensor->set_ae_level(_sensor, dsp_level);
}

// Configure the gain ceiling
void CameraHandler::setGainCeiling(gainceiling_t gain)
{
    if(_sensor == NULL) return;
    _sensor->set_gainceiling(_sensor, gain);
}

// Get last frame width
int CameraHandler::getWidth()
{
    if (_sensor == NULL) return 0;
    switch (_sensor->status.framesize)
    {
        case FRAMESIZE_QQVGA: return 160;
        case FRAMESIZE_QVGA: return 320;
        case FRAMESIZE_CIF: return 400;
        case FRAMESIZE_VGA: return 640;
        case FRAMESIZE_SVGA: return 800;
        case FRAMESIZE_XGA: return 1024;
        case FRAMESIZE_SXGA: return 1280;
        case FRAMESIZE_UXGA: return 1600;
        case FRAMESIZE_QXGA: return 2048;
        default: return 0;
    }
}

// Get last frame height
int CameraHandler::getHeight()
{
    if (_sensor == NULL) return 0;
    switch (_sensor->status.framesize)
    {
        case FRAMESIZE_QQVGA: return 120;
        case FRAMESIZE_QVGA: return 240;
        case FRAMESIZE_CIF: return 296;
        case FRAMESIZE_VGA: return 480;
        case FRAMESIZE_SVGA: return 600;
        case FRAMESIZE_XGA: return 768;
        case FRAMESIZE_SXGA: return 1024;
        case FRAMESIZE_UXGA: return 1200;
        case FRAMESIZE_QXGA: return 1536;
        default: return 0;
    }
}

// Get last frame pixel format
pixformat_t CameraHandler::getPixelFormat()
{
    if (_sensor == NULL) return PIXFORMAT_JPEG;
    return _sensor->pixformat;
}

// Get the current working mode
CameraHandler::Camera_Mode CameraHandler::getCurrentMode()
{
    return _current_mode;
}

// Special effect aplication tool
void CameraHandler::setSpecialEffect(Special_Effect effect)
{
    if (_sensor == NULL) return;
    _sensor->set_special_effect(_sensor, (int)effect);
}

// Test mode tool
void CameraHandler::setColorBar(bool enable)
{
    if (_sensor == NULL) return;
    _sensor->set_colorbar(_sensor, enable ? 1 : 0);
}

// Image converter tool
bool CameraHandler::convertFrameToJpeg(camera_fb_t* original, uint8_t **out_buf, size_t* out_len)
{
    if (original == NULL || out_buf == NULL || out_len == NULL) return false;
    return frame2jpg(original, 80, out_buf, out_len);
}

// Apply OV3660 configuration corrections
void CameraHandler::_applySensorSettings()
{
    if (_sensor == NULL) return;
    if (_sensor->id.PID == OV3660_PID)
    {
        _sensor->set_vflip(_sensor, 1); // Usually the sensor camera are flipped vertically
        _sensor->set_saturation(_sensor, -2); // Lower saturation, more real colors
        _sensor->set_brightness(_sensor, 1); // A bit high brightness for indoor
        _sensor->set_whitebal(_sensor, 1); // White balance adejustment
        _sensor->set_awb_gain(_sensor, 1); // Apply the AWB gain to see White Balance changes
        _sensor->set_wb_mode(_sensor, 0); // Mode 0 = Auto
    }
}

// Apply specific configuration by mode selected
void CameraHandler::_configureCameraByMode(camera_config_t &config, Camera_Mode mode)
{
    switch (mode)
    {
        case MODE_AI: // Configuration to look for stability (better for TinyML)
            config.pixel_format = PIXFORMAT_RGB565;
            config.frame_size = FRAMESIZE_QVGA;
            config.fb_count = 1;
            config.fb_location = CAMERA_FB_IN_DRAM;
            config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
            config.xclk_freq_hz = 10000000;
            break;
        case MODE_GRAYSCALE: // Configuration for quick movement detection or QR scans
            config.pixel_format = PIXFORMAT_GRAYSCALE;
            config.frame_size = FRAMESIZE_QVGA;
            config.fb_count = 1;
            config.fb_location = CAMERA_FB_IN_DRAM;
            config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
            config.xclk_freq_hz = 20000000;
            break;
        case MODE_HIGH_RES: // Configuration to store images** (Need test)
            config.pixel_format = PIXFORMAT_JPEG;
            config.frame_size = FRAMESIZE_UXGA;
            config.jpeg_quality = 10;
            config.fb_count = 1;
            if (psramFound()) config.fb_location = CAMERA_FB_IN_PSRAM;
            else {
                config.frame_size = FRAMESIZE_SVGA;
                config.fb_location = CAMERA_FB_IN_DRAM;
            }
            config.grab_mode = CAMERA_GRAB_LATEST;
            config.xclk_freq_hz = 20000000;
            break;
        case MODE_STREAMING: // Default configuration, for viedo streaming via web server
        default:
            config.pixel_format = PIXFORMAT_JPEG;
            config.frame_size = FRAMESIZE_CIF;
            config.jpeg_quality = 15;
            if (psramFound())
            {
                config.fb_count = 2;
                config.fb_location = CAMERA_FB_IN_PSRAM;
            } else {
                config.fb_count = 1;
                config.fb_location = CAMERA_FB_IN_DRAM;
            }
            config.grab_mode = CAMERA_GRAB_LATEST;
            config.xclk_freq_hz = 20000000;
            break;
    }
}
