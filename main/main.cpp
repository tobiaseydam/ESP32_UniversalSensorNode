#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "usn_storage.hpp"
#include "usn_wifi.hpp"
#include "usn_stateMashine.hpp"
#include "usn_http.hpp"
#include "usn_lcd.hpp"

cSPIFFSManager* spiffs = new cSPIFFSManager();
cHttp* http = new cHttp();
cWiFi* wifi = new cWiFi(http);
cStateMashine* sm;

extern "C" {
    void app_main(void);
}

void app_main(){
    display_message_t* dm1 = new display_message_t("Test","Ficken");
    dm1->printToSerial();
    display_buffer_t* db = new display_buffer_t();
    db->enqueue(dm1);

    display_t::lcd_init();
    TaskHandle_t xDisplayTaskHandle = NULL;

    xTaskCreate(display_t::run, "DISPLAY", 2048, db, tskIDLE_PRIORITY, &xDisplayTaskHandle);

    spiffs->init();
    wifi->init();
    ESP_LOGI("TAG", "%p", (void *)http);
    sm = new cStateMashine(spiffs, wifi);
    sm->begin(NULL);
}