#ifndef WEB_SERVER_HANDLER_H
#define WEB_SERVER_HANDLER_H

#include <Arduino.h>
#include "esp_http_server.h"
#include "CameraHandler.h"

// This is the definition of the type of function for the Callbacks for commands
// Receives: (command_name, numeric_value)
typedef std::function<void(String, int)> CommandCallback;

class WebServerHandler {

    public:

        // Constructor
        WebServerHandler(void);

        // Starts the Web server (default port 80)
        void begin();
        // Stops the Web server
        void stop();

        // Gives access to the web server to the camera
        void enableCamera(CameraHandler& Camera);
        // Gives the content for an interface to deploy with the web server
        void setUserInterface(const char* html_content);
        // Link a function to process the command received in the web server
        void setCommandCallback(CommandCallback callback);
        // Updates a status in the web server
        void setStatus(String msg);

    private:

        // Pointers to other tools
        httpd_handle_t _httpd_web = NULL;
        httpd_handle_t _httpd_stream = NULL;
        CameraHandler* _camera_ptr = nullptr;
        CommandCallback _callback = nullptr;
        const char* _index_html;

        // Static paths to handle esp_http_server
        static esp_err_t indexHandler(httpd_req_t* req);
        static esp_err_t streamHandler(httpd_req_t* req);
        static esp_err_t cmdHandler(httpd_req_t* req);

        // Function to configure and start the streaming server
        void _startStreamServer();

        // Pointer to the own instance to access variables from static function
        static WebServerHandler* _instance;

        // Variables for status update
        String _status_msg;
        static esp_err_t statusHandler(httpd_req_t* req);

};

#endif
