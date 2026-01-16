#include "MicHandler.h"

/**
 * @brief Constructor
 */
MicHandler::MicHandler()
{
    _initialized = false;
}

/**
 * @brief Initialize I2S hardware in PDM mode.
 * @return True when initialization was OK, False otherwise.
 */
bool MicHandler::begin()
{
    if (_initialized) return true;
    // Configure L/R GPIO
    pinMode(PIN_MIC_LR, OUTPUT);
    digitalWrite(PIN_MIC_LR, LOW);
    // I2S Configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = MIC_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // Stereo
        .communication_format = I2S_COMM_FORMAT_STAND_PCM_SHORT,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    // Pin mapping
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = PIN_MIC_CLK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_MIC_DATA
    };
    if (i2s_driver_install(MIC_I2S_PORT, &i2s_config, 0, NULL) != ESP_OK) return false;
    if (i2s_set_pin(MIC_I2S_PORT, &pin_config) != ESP_OK) return false;
    _initialized = true;
    return true;
}

/**
 * @brief Stops the I2S to free resources.
 */
void MicHandler::end()
{
    if (_initialized)
    {
        i2s_driver_uninstall(MIC_I2S_PORT);
        _initialized = false;
    }
}

/**
 * @brief Read raw audio samples.
 * @param buffer Array to store data.
 * @param samples Number of samples to be readen.
 * @return Number of samples stored in the array.
 */
size_t MicHandler::read(int16_t* buffer, size_t samples)
{
    if (!_initialized) return 0;
    size_t bytes_read = 0;
    // We read the double quantity of samples because the internal buffer is stereo (L+R)
    // and later we process the samples to return ONLY one channel if needed, but because
    // efficiency, this driver returns raw data.
    i2s_read(MIC_I2S_PORT, buffer, samples * sizeof(int16_t), &bytes_read, 100);
    return bytes_read / sizeof(int16_t);
}
