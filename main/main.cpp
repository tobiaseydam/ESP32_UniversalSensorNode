#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "usn_storage.hpp"
#include "usn_wifi.hpp"
#include "usn_http.hpp"
#include "usn_lcd.hpp"

display_buffer_t* display_buffer = new display_buffer_t();
wifi_adapter* wifi = new wifi_adapter();
storage_adapter* stor = new storage_adapter();
http_server* http = new http_server();

extern "C" {
    void app_main(void);
}

void app_main(){
    esp_log_level_set("display_message_t", ESP_LOG_VERBOSE);
    esp_log_level_set("display_buffer_t", ESP_LOG_VERBOSE);
    esp_log_level_set("display_t", ESP_LOG_VERBOSE);
    
    stor->init();
    wifi->_init();
    http->set_storage_adapter(stor);

    display_t::lcd_init();
    TaskHandle_t xDisplayTaskHandle = NULL;

    xTaskCreate(display_t::run, "DISPLAY", 2048, display_buffer, 
        tskIDLE_PRIORITY, &xDisplayTaskHandle);

    
    stor->set_display_buffer(display_buffer);

    wifi->set_display_buffer(display_buffer);
    wifi->set_http_server(http);
    wifi->start_station(stor->get_global_value(WIFISSID),
        stor->get_global_value(WIFIPASS));
}