#ifndef USN_WIFI_H
    #define USN_WIFI_H

    #include <stdint.h>
    #include "esp_event_loop.h"
    #include "freertos/event_groups.h"

    #include "usn_http.hpp"
    #include "usn_lcd.hpp"

    #define STA_CONN_MAX_RETRIES 5
    #define AP_NAME "ESP32_test"
    #define AP_PASS "k79Zqr2LjOOd"

    class wifi_adapter{
        private:
            static constexpr char *TAG = (char*)"wifi_adapter";
            static display_buffer_t* _db;
            static http_server* _http;
            static int _sta_conn_max_retries;
            static int _sta_conn_curr_retry;
                        
            static esp_err_t _event_handler(void *ctx, system_event_t *event);
        public:
            static void _init();
            static void start_softap(const char* ssid, const char* pass);
            static void start_station(const char* ssid, const char* pass);
            
            static void set_display_buffer(display_buffer_t* db);
            static void set_http_server(http_server* http);
    };

#endif