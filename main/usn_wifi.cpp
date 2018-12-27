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


cHttp* cWiFi::_http = NULL;

cWiFi::cWiFi(cHttp* http){
    memset(&cWiFi::_http, 0, sizeof(cWiFi::_http));  
    cWiFi::_http = http;
}

esp_err_t cWiFi::_event_handler(void *ctx, system_event_t *event){
    httpd_handle_t *server = (httpd_handle_t *) ctx;
    //ESP_LOGI(TAG, "%p", (void *)_http);
    //_http->dummy();
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
	        ESP_ERROR_CHECK(esp_wifi_connect());
	        break;
        case SYSTEM_EVENT_STA_GOT_IP:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
	        ESP_LOGI(TAG, "got ip:%s\n",
		        ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            //cLCD::lcd_clear();
            //cLCD::lcd_setcursor( 0, 1 );
            //cLCD::lcd_string("SYS: STA_GOT_IP");
            //char ip[16];
            //sprintf(ip, "%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            //cLCD::lcd_setcursor( 0, 2 );
            //cLCD::lcd_string(ip);
            if(server == NULL){
                server = _http->start_webserver();
            }
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            //cLCD::lcd_clear();
            //cLCD::lcd_setcursor( 0, 1 );
            //cLCD::lcd_string("SYS: STA_DISCONN");
            
	        //ESP_ERROR_CHECK(esp_wifi_connect());
            if(server != NULL){
                cHttp::stop_webserver(*server);
                server = NULL;
            }
	        break;
        case SYSTEM_EVENT_AP_START:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_AP_START");
            //cLCD::lcd_clear();
            //cLCD::lcd_setcursor( 0, 1 );
            //cLCD::lcd_string("SYS: AP_START");
            //char ip2[16];
            //sprintf(ip2, "192.168.1.4");
            //cLCD::lcd_setcursor( 0, 2 );
            //cLCD::lcd_string(ip2);
            
            if(server == NULL){
                server = _http->start_webserver();
            }
            break;
        case SYSTEM_EVENT_AP_STOP:
	        ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STOP");
            //cLCD::lcd_clear();
            //cLCD::lcd_setcursor( 0, 1 );
            //cLCD::lcd_string("SYS: AP_STOP");
            
            if(server == NULL){
                server = _http->start_webserver();
            }
            break;
        default:
            break;
        }
    return ESP_OK;
}

void cWiFi::init(){
    ESP_LOGI(TAG, "WiFi initializing...");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //ESP_LOGI(TAG, "%p", (void *)cWiFi::_http);
    //ESP_LOGI(TAG, "%p", (void *)_http);
}

void cWiFi::disconnect(){
    ESP_ERROR_CHECK(esp_wifi_stop());
    init();
};

void cWiFi::init_softap(const char* ssid, const char* pass){
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(_event_handler, NULL));

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

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             ssid, pass);
}

void cWiFi::init_sta(const char* ssid, const char* pass){
    wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));  
    memcpy(wifi_config.sta.ssid, ssid, strlen(ssid)+1);
    memcpy(wifi_config.sta.password, pass, strlen(pass)+1);
    wifi_config.sta.bssid_set = false;

    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}