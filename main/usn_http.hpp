#ifndef USN_HTTP_HPP
    #define USN_HTTP_HPP

    #include "esp_http_server.h"

    #define MAX_PARAMS 8

    typedef struct paramset{
        char params[MAX_PARAMS][16]; 
        char vals[MAX_PARAMS][32]; 
    } paramset_t;

    class cHttp{
        private:
            static httpd_uri_t uri_get;
            static httpd_uri_t spiffs_get;
            static constexpr char *TAG = (char*)"HTTP";
            static paramset_t* _parseURI(httpd_req_t *req);
        public:
            cHttp();
            static httpd_handle_t* start_webserver();
            static void stop_webserver(httpd_handle_t server);
            static esp_err_t get_handler(httpd_req_t *req);
            static esp_err_t spiffs_get_handler(httpd_req_t *req);
            void dummy();
    };

#endif