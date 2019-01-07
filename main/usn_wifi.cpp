#include "usn_wifi.hpp"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "usn_storage.hpp"
#include "usn_lcd.hpp"

#include <string.h>

//----- wifi_adapter -----------------------------------------

display_buffer_t* wifi_adapter::_db = NULL;
http_server* wifi_adapter::_http = NULL;

int wifi_adapter::_sta_conn_max_retries = STA_CONN_MAX_RETRIES;
int wifi_adapter::_sta_conn_curr_retry = 0;

void wifi_adapter::_init(){
    ESP_LOGD(TAG, "WiFi initializing...");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(_event_handler, NULL));
}


esp_err_t wifi_adapter::_event_handler(void *ctx, system_event_t *event){
    switch(event->event_id) {

        case SYSTEM_EVENT_STA_START:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            if(_db != NULL){
                display_message_t* m = new display_message_t("STA_START", "");
                _db->enqueue(m);
            }
	        ESP_ERROR_CHECK(esp_wifi_connect());
	        break;

        case SYSTEM_EVENT_STA_CONNECTED:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
            if(_db != NULL){
                display_message_t* m = new display_message_t("STA_CONNECTED", "");
                _db->enqueue(m);
            }
	        break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            if(_sta_conn_curr_retry<_sta_conn_max_retries){
                if(_db != NULL){
                    display_message_t* m = new display_message_t("STA_DISCONNECTED", "Retry");
                    _db->enqueue(m);
                }
                _sta_conn_curr_retry++;
	            ESP_ERROR_CHECK(esp_wifi_connect());
            }else{
                if(_db != NULL){
                    display_message_t* m = new display_message_t("STA_DISCONNECTED", "Stop");
                    _db->enqueue(m);
                }
                start_softap(AP_NAME, AP_PASS);
            }
	        break;

        case SYSTEM_EVENT_STA_GOT_IP:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
	        ESP_LOGI(TAG, "got ip:%s\n",
		        ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            _sta_conn_curr_retry = 0;
            if(_db != NULL){
                display_message_t* m = new display_message_t("STA_GOT_IP", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
                _db->enqueue(m);
            }
            if(_http != NULL){
                _http->start_webserver();
            }
            break;
        
        case SYSTEM_EVENT_AP_START:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_AP_START");
            if(_db != NULL){
                display_message_t* m = new display_message_t("AP_START", AP_NAME);
                _db->enqueue(m);
            }
	        break;

        default:
            break;
    }
    return ESP_OK;
}

void wifi_adapter::start_softap(const char* ssid, const char* pass){
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));  
    memcpy(wifi_config.ap.ssid, ssid, strlen(ssid));
    wifi_config.ap.ssid_len = strlen(ssid);
    memcpy(wifi_config.ap.password, pass, strlen(pass));
    wifi_config.ap.max_connection = 5;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_adapter::start_station(const char* ssid, const char* pass){
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));  
    memcpy(wifi_config.sta.ssid, ssid, strlen(ssid)+1);
    memcpy(wifi_config.sta.password, pass, strlen(pass)+1);
    wifi_config.sta.bssid_set = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_adapter::set_display_buffer(display_buffer_t* db){
    wifi_adapter::_db = db;
}

void wifi_adapter::set_http_server(http_server* http){
    wifi_adapter::_http = http;
}
