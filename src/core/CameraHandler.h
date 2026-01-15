#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include "esp_camera.h"
#include "CameraPins.h"
#include <Arduino.h>

class CameraHandler {

    public:

        enum Camera_Mode {
            MODE_STREAMING,
            MODE_AI,
            MODE_GRAYSCALE,
            MODE_HIGH_RES,
        };

        enum Special_Effect {
            NO_EFFECT = 0,
            EFFECT_NEGATIVE = 1,
            EFFECT_GRAYSCALE = 2,
            EFFECT_RED_TINT = 3,
            EFFECT_GREEN_TINT = 4,
            EFFECT_BLUE_TINT = 5,
            EFFECT_SEPIA = 6
        };

        // Constructor
        CameraHandler();

        // Initialize the Camera
        bool init(Camera_Mode mode = MODE_AI);
        // Get the current initialization status
        bool isInitialized();

        // Get a frame (a picture)
        camera_fb_t* getFrame();
        // Free the RAM from the last frame
        void releaseFrame(camera_fb_t* fb);
        // Change the image mode
        void setMode(Camera_Mode mode);

        // Configure the image resolution
        void setResolution(framesize_t size);
        // Configure the image quality
        void setQuality(int quality);
        // Configure the Vertical Image Flip
        void setVFlip(bool enable);
        // Configure the Horizontal Image Mirror
        void setHMirror(bool enable);
        // Configure the image Brightness
        void setBrightness(int level);
        // Configure the image saturation
        void setSaturation(int level);
        // Configure the image contrast
        void setContrast(int level);
        // Configure the image White Balance
        void setWhiteBalance(bool enable, int mode = 0);
        // Configure the image exposure control
        void setExposureControl(bool enable, int dsp_level = -1);
        // Configure the gain ceiling
        void setGainCeiling(gainceiling_t gain);

        // Get last frame width
        int getWidth();
        // Get last frame height
        int getHeight();
        // Get last frame pixel format
        pixformat_t getPixelFormat();
        // Get the current working mode
        Camera_Mode getCurrentMode();

        // Special effect aplication tool
        void setSpecialEffect(Special_Effect effect);
        // Test mode tool
        void setColorBar(bool enable);
        // Image converter tool
        bool convertFrameToJpeg(camera_fb_t* original, uint8_t** out_buf, size_t* out_len);

    private:

        bool _is_initialized;
        Camera_Mode _current_mode;
        sensor_t* _sensor;

        // Apply OV3660 configuration corrections
        void _applySensorSettings();
        // Apply specific configuration by mode selected
        void _configureCameraByMode();

};

#endif
