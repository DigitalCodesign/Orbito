#include "WebServerHandler.h"

// Initialize the static pointer
WebServerHandler* WebServerHandler::_instance = nullptr;

const char* DEFAULT_HTML = "<h1>Server Active</h1><p>Use .setUserInterface() to customize.</p>";

// Constructor
WebServerHandler::WebServerHandler()
{
    _camera_ptr = nullptr;
    _index_html = DEFAULT_HTML;
    _instance = this;
    _status_msg = "Online";
}

// Starts the Web server (default port 80)
void WebServerHandler::begin()
{
    // Get the config of the web server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.stack_size = 4096;
    config.max_uri_handlers = 8;
    // Starts the web server
    if (httpd_start(&_httpd_web, &config) == ESP_OK)
    {
        // Set each server details
        static httpd_uri_t index_uri = { // Main path (UI)
            .uri = "/",
            .method = HTTP_GET,
            .handler = indexHandler,
            .user_ctx = NULL
        };
        static httpd_uri_t cmd_uri = { // Commands path (API)
            .uri = "/cmd",
            .method = HTTP_GET,
            .handler = cmdHandler,
            .user_ctx = NULL
        };
        static httpd_uri_t stream_uri = { // Streaming path (Only when camera have been given)
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = streamHandler,
            .user_ctx = NULL
        };
        static httpd_uri_t status_uri = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = statusHandler,
            .user_ctx = NULL
        };
        // Register each path with the handlers
        httpd_register_uri_handler(_httpd_web, &index_uri);
        httpd_register_uri_handler(_httpd_web, &cmd_uri);
        httpd_register_uri_handler(_httpd_web, &status_uri);
        _startStreamServer();
    }
}

void WebServerHandler::_startStreamServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 81;
    config.ctrl_port = 32769;
    config.stack_size = 8192;
    if (httpd_start(&_httpd_stream, &config) == ESP_OK) {
        static httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = streamHandler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(_httpd_stream, &stream_uri);
    }
}

// Stops the Web server
void WebServerHandler::stop()
{
    if (_httpd_web) httpd_stop(_httpd_web);
    if (_httpd_stream) httpd_stop(_httpd_stream);
}

// Gives access to the web server to the camera
void WebServerHandler::enableCamera(CameraHandler &Camera)
{
    _camera_ptr = &Camera;
}

// Gives the content for an interface to deploy with the web server
void WebServerHandler::setUserInterface(const char *html_content)
{
    _index_html = html_content;
}

// Link a function to process the command received in the web server
void WebServerHandler::setCommandCallback(CommandCallback callback)
{
    _callback = callback;
}

// Updates a status in the web server
void WebServerHandler::setStatus(String msg)
{
    _status_msg = msg;
}

esp_err_t WebServerHandler::indexHandler(httpd_req_t *req)
{
    if (!_instance || !_instance->_index_html) return ESP_FAIL;
    httpd_resp_send(req, _instance->_index_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WebServerHandler::streamHandler(httpd_req_t *req)
{
    if (!_instance || !_instance->_camera_ptr)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    CameraHandler* camera = _instance->_camera_ptr;
    camera_fb_t* fb = NULL;
    esp_err_t response = ESP_OK;
    char part_buffer[64];
    // Standar MJPEG header
    response = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
    if (response != ESP_OK) return response;
    // Streaming loop
    while (true)
    {
        fb = camera->getFrame();
        if (!fb)
        {
            response = ESP_FAIL;
            break;
        }
        // Convert frame to format RGB
        uint8_t* data_buffer = fb->buf;
        size_t data_length = fb->len;
        uint8_t*jpg_buffer = NULL;
        size_t jpg_length = 0;
        bool converted = false;
        if (fb->format != PIXFORMAT_JPEG)
        {
            if (camera->convertFrameToJpeg(fb, &jpg_buffer, &jpg_length))
            {
                data_buffer = jpg_buffer;
                data_length = jpg_length;
                converted = true;
            }
        }
        // Send the frame header
        size_t header_length = snprintf((char*)part_buffer, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", data_length);
        response = httpd_resp_send_chunk(req, (const char*)part_buffer, header_length);
        // Send the data
        if (response == ESP_OK) response = httpd_resp_send_chunk(req, (const char*)data_buffer, data_length);
        // Send frame end
        if (response == ESP_OK) response = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 13);
        // Clean memory
        if (converted) free(jpg_buffer);
        camera->releaseFrame(fb);
        if (response != ESP_OK) break;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return response;
}

esp_err_t WebServerHandler::cmdHandler(httpd_req_t *req)
{
    if (!_instance) return ESP_FAIL;
    // Parse the URL looking for params (URL must be: /cmd?id=command_name&value=a_number)
    char* buffer;
    size_t buffer_length;
    // Get the length of the query string
    buffer_length = httpd_req_get_url_query_len(req) + 1;
    if (buffer_length > 1)
    {
        buffer = (char*)malloc(buffer_length);
        if (httpd_req_get_url_query_str(req, buffer, buffer_length) == ESP_OK)
        {
            char param_id[32] = { 0 };
            char param_value[16] = { 0 };
            // Extract the "id" (command)
            if (httpd_query_key_value(buffer, "id", param_id, sizeof(param_id)) == ESP_OK)
            {
                // Extract the "value" (optional)
                int value = 0;
                if (httpd_query_key_value(buffer, "value", param_value, sizeof(param_value)) == ESP_OK)
                    value = atoi(param_value);
                // Trigger the callback given
                if (_instance->_callback) _instance->_callback(String(param_id), value);
            }
        }
        free(buffer);
    }
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

esp_err_t WebServerHandler::statusHandler(httpd_req_t* req)
{
    if (!_instance) return ESP_FAIL;
    httpd_resp_send(req, _instance->_status_msg.c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
