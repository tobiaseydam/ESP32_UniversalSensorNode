#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "usn_storage.hpp"
#include "usn_wifi.hpp"
#include "usn_stateMashine.hpp"
#include "usn_http.hpp"

cSPIFFSManager* spiffs = new cSPIFFSManager();
cHttp* http = new cHttp();
cWiFi* wifi = new cWiFi(http);
cStateMashine* sm;

extern "C" {
    void app_main(void);
}

void app_main(){
    spiffs->init();
    wifi->init();
    ESP_LOGI("TAG", "%p", (void *)http);
    sm = new cStateMashine(spiffs, wifi);
    sm->begin(NULL);
}