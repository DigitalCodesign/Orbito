#ifndef ORBITO_AI_H
#define ORBITO_AI_H

#include <Arduino.h>

// Ensure the Edge Impulse model library is included first
#if !defined(EI_CLASSIFIER_LABEL_COUNT)
    #error "CRITICAL ERROR: You must include your Edge Impulse model library BEFORE AIHandler.h"
#endif

// Calculate expected DSP size for Grayscale (Width * Height * 1)
#define AI_HANDLER_EXPECTED_GRAY (EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT)

// Automatically detect if the model expects Grayscale or RGB based on DSP input frame size
#if EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE == AI_HANDLER_EXPECTED_GRAY
    #define AI_HANDLER_IS_GRAYSCALE 1
#else
    #define AI_HANDLER_IS_GRAYSCALE 0
#endif

// Check if the model is for Vision (Camera) or Sensors (Data)
#if defined(EI_CLASSIFIER_SENSOR) && EI_CLASSIFIER_SENSOR == EI_CLASSIFIER_SENSOR_CAMERA
    #define AI_HANDLER_VISION_MODE 1
    #include "esp_camera.h"
#else
    #define AI_HANDLER_VISION_MODE 0
#endif

#include "./core/AIInterface.h"
#include <edge-impulse-sdk/classifier/ei_run_classifier.h>

// Pointers used to bridge C++ Class data to C-style callbacks.
// Marked 'volatile' to prevent compiler optimization issues.
static volatile float* _ai_raw_buf = NULL;

#if AI_HANDLER_VISION_MODE
    static volatile camera_fb_t* _cam_fb = NULL;
    static bool _ai_swap_bytes = true; 
#endif

/**
 * @brief Callback for Generic Sensors (Audio, Accelerometer, etc.)
 * Copies raw float data from the user buffer to the Edge Impulse DSP buffer.
 */
static int _ai_raw_callback(size_t offset, size_t length, float* output_pointer) {
    if (!_ai_raw_buf) return -1;
    memcpy(output_pointer, (float*)_ai_raw_buf + offset, length * sizeof(float));
    return 0;
}

/**
 * @brief Callback for Vision (Camera)
 * Performs on-the-fly resizing, cropping, color conversion, and resizing
 * to feed the Neural Network without allocating large intermediate buffers.
 */
#if AI_HANDLER_VISION_MODE
    static int _ai_image_callback(size_t offset, size_t length, float* output_pointer) {
        if (!_cam_fb) return -1;
        size_t output_pointer_ix = 0;
        int target_w = EI_CLASSIFIER_INPUT_WIDTH;
        int target_h = EI_CLASSIFIER_INPUT_HEIGHT;
        while (length > 0) {
            // --- Coordinate Calculation ---
            // Determine which pixel (and channel) we are processing
            #if AI_HANDLER_IS_GRAYSCALE
                size_t pixel_index = offset + output_pointer_ix;
            #else
                size_t pixel_index = (offset + output_pointer_ix) / 3;
                size_t channel = (offset + output_pointer_ix) % 3; // 0=R, 1=G, 2=B
            #endif
            size_t x = pixel_index % target_w;
            size_t y = pixel_index / target_w;
            // --- Resize (Nearest Neighbor) ---
            // Map target coordinates back to source camera coordinates
            size_t origin_x = (x * _cam_fb->width) / target_w;
            size_t origin_y = (y * _cam_fb->height) / target_h;
            // Safety Clamp
            if (origin_x >= _cam_fb->width) origin_x = _cam_fb->width - 1;
            if (origin_y >= _cam_fb->height) origin_y = _cam_fb->height - 1;
            // Calculate index in raw RGB565 camera buffer (2 bytes per pixel)
            size_t raw_ix = (origin_y * _cam_fb->width + origin_x) * 2;
            // Read Bytes & Swap Endianness (ESP32 specific correction)
            uint8_t b1 = _cam_fb->buf[raw_ix];
            uint8_t b2 = _cam_fb->buf[raw_ix + 1];
            uint16_t rgb565 = _ai_swap_bytes ? ((b1 << 8) | b2) : ((b2 << 8) | b1);
            // Extract RGB components (Range 0-255)
            float r = ((rgb565 >> 11) & 0x1F) * 255.0f / 31.0f;
            float g = ((rgb565 >> 5) & 0x3F) * 255.0f / 63.0f;
            float b = (rgb565 & 0x1F) * 255.0f / 31.0f;
            float val = 0.0f;
            // --- Color Conversion ---
            #if AI_HANDLER_IS_GRAYSCALE
                // Convert to Luminance (Grayscale) using standard weights
                val = (0.299f * r) + (0.587f * g) + (0.114f * b);
            #else
                // RGB Mode: Return the specific channel requested by the loop
                if (channel == 0) val = r;
                else if (channel == 1) val = g;
                else val = b;
            #endif
            // Assign to output
            output_pointer[output_pointer_ix++] = val;
            length--;
        }
        return 0;
    }
#endif

// ==========================================================================
// 4. MAIN CLASS: AIHandler
// ==========================================================================
class OrbitoAI : public AIInterface
{

    public:

        // Public result object for advanced access
        ei_impulse_result_t result;

        AIHandler() {}
        
        /**
         * @brief Run inference on generic sensor data (Float array)
         * @param data Pointer to the float array
         * @param data_size Size of the array (must match EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
         * @return true if inference successful, false otherwise
         */
        bool predict(float* data, size_t data_size)
        {
            if (!data || data_size != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) return false;
            _ai_raw_buf = data;
            signal_t signal;
            signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
            signal.get_data = &_ai_raw_callback;
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
            _ai_raw_buf = NULL; // Cleanup
            return (res == EI_IMPULSE_OK);
        }
        #if AI_HANDLER_VISION_MODE
        /**
         * @brief Run inference on a Camera Frame
         * @param frame Pointer to the camera_fb_t struct
         * @return true if inference successful, false otherwise
         */
        bool predict(camera_fb_t* frame)
        {
            if (!frame) return false;
            _cam_fb = frame;
            signal_t signal;
            signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
            signal.get_data = &_ai_image_callback;
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
            _cam_fb = NULL; // Cleanup
            return (res == EI_IMPULSE_OK);
        }
        
        /**
         * @brief Fix inverted colors (Red appearing as Blue)
         * @param fix true to swap bytes (default for ESP32), false to disable
         */
        void fixColors(bool fix) { _ai_swap_bytes = fix; }
        #endif

        /**
         * @brief Get the label of the class with the highest probability
         */
        const char* getLabel()
        {
            int best_idx = -1; float best_val = -1.0;
            for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
            {
                if (result.classification[i].value > best_val) {
                    best_val = result.classification[i].value;
                    best_idx = i;
                }
            }
            return (best_idx > -1) ? result.classification[best_idx].label : "?";
        }

        /**
         * @brief Get the confidence score (0.0 - 1.0) of the highest class
         */
        float getConfidence()
        {
            float best_val = 0.0;
            for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
                if (result.classification[i].value > best_val) best_val = result.classification[i].value;
            return best_val;
        }

};

#endif
