#ifndef USN_WIFI_H
    #define USN_WIFI_H

    #include <stdint.h>
    #include "esp_event_loop.h"
    #include "freertos/event_groups.h"

    #include "usn_http.hpp"

    

    class cWiFi {
        private:
            static constexpr char *TAG = (char*)"WiFi";
            EventGroupHandle_t wifi_event_group;
            static esp_err_t _event_handler(void *ctx, system_event_t *event);
            static cHttp* _http;
        public:
            cWiFi(cHttp* http);
            void init();
            void init_softap(const char* ssid, const char* pass);
            void init_sta(const char* ssid, const char* pass);
            void scan();
    };

#endif