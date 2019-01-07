#ifndef USN_HTTP_HPP
    #define USN_HTTP_HPP

    #include "esp_http_server.h"
    #include "usn_storage.hpp"

    #define MAX_PARAMS 8

    typedef struct paramset{
        char params[MAX_PARAMS][16]; 
        char vals[MAX_PARAMS][32]; 
        uint8_t num_sets = 0;
    } paramset_t;

    class http_server{
        private:
            static httpd_uri_t root_get;
            static httpd_uri_t spiffs_get;
            static httpd_uri_t upload_post;
            static constexpr char *TAG = (char*)"http_server";
            static paramset_t* _parseURI(httpd_req_t *req);
            static storage_adapter* _sa;
        public:
            http_server();
            static httpd_handle_t* start_webserver();
            static void stop_webserver(httpd_handle_t server);
            static esp_err_t get_handler(httpd_req_t *req);
            static esp_err_t spiffs_get_handler(httpd_req_t *req);
            static esp_err_t upload_post_handler(httpd_req_t *req);
            static void set_storage_adapter(storage_adapter* sa);
    };

#endif