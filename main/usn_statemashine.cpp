#include "usn_stateMashine.hpp"

#include "esp_log.h"
#include "usn_lcd.hpp"

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

bool cStateMashine::btn_val[7];
uint8_t cStateMashine::btn_pin[7] = {BTN1, BTN2, BTN3, BTN4, BTN5, BTN6, BTN7};

cStateMashine::cStateMashine(cSPIFFSManager* spiffsManager, cWiFi* wifi){
    _spiffs = spiffsManager;
    _wifi = wifi;
}

void cStateMashine::begin(void *pvParameter){
    _nextStep();
}

void cStateMashine::_nextStep(){
    while(_runningState!=PAUSED){
        switch(_state){
            case START:  
                _start();
                break;
            case WIFI_LOOK_FOR_DATA:
                _wifiLookForData();
                break;
            case WIFI_LOGIN:
                _wifiLogin();
                break;
            case UNRESOLVABLE_ERROR:
                _unresolvableError();
                break;
        }
    }
}

void cStateMashine::_start(){
    ESP_LOGI(TAG, "entering: sm_start");
    cGPIO::setPinMode(13, OUTPUT);
    // Turn on LCD
    cGPIO::setPin(13, 1);
    cLCD::lcd_init();

    for(uint8_t i = 0; i<7; i++){
        cGPIO::setPinMode(btn_pin[i], INPUT);
        cGPIO::setPin(btn_pin[i], 1);
    }

    _runningState = RUNNING;
    _state = WIFI_LOOK_FOR_DATA;
}

void cStateMashine::_wifiLookForData(){
    ESP_LOGI(TAG, "entering: sm_wifi_look_for_data");
    if (true || _spiffs->fileExists(WIFI_FILE)){
        if (true){
            _state = WIFI_LOGIN;
            return;
        }else{
            //_state = WIFI_OPEN_ACCESSPOINT;
            return;
        }
    }else{
        //_state = WIFI_OPEN_ACCESSPOINT;
        return;
    }
    //_state = UNRESOLVABLE_ERROR;
    return;
}

void cStateMashine::_wifiLogin(){
    ESP_LOGI(TAG, "entering: sm_wifi_login");


    char* ssid = new char[32];
    cSPIFFSManager::getSpecialValue(WIFI_SSID, ssid);
    char* pass = new char[32];
    cSPIFFSManager::getSpecialValue(WIFI_PASS, pass);
    ESP_LOGI(TAG, "connectiong to:");
    ESP_LOGI(TAG, "  SSID: %s", ssid);
    _wifi->init_sta(ssid, pass);
    //_wifi->init_softap("TEST", "ABCDEFGH");
    _state = UNRESOLVABLE_ERROR;
    //_state = MQTT_LOOK_FOR_DATA;
    
    return;
}

void cStateMashine::_readBtn(void *pvParameters){
    while(true){
        for(uint8_t i = 0; i<7; i++){
            bool val = cGPIO::getPin(btn_pin[i]) == 0;
            if(btn_val[i]!=val){
                btn_val[i] = val;
                ESP_LOGI("Button", "BTN %d = %d", i, val);
                vTaskDelay( 500/ portTICK_PERIOD_MS);
            }
        }
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed=1;
    TIMERG0.wdt_wprotect=0;
    }
}

void cStateMashine::_unresolvableError(){
    ESP_LOGI(TAG, "THE END");

    xTaskCreate(_readBtn, "_readBtn", 2048, NULL, 10, NULL);

    vTaskDelay( 10000/ portTICK_PERIOD_MS);
}