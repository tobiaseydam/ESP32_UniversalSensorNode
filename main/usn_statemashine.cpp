#include "usn_stateMashine.hpp"

#include "esp_log.h"

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

void cStateMashine::_unresolvableError(){
    ESP_LOGI(TAG, "THE END");
    vTaskDelay( 1000/ portTICK_PERIOD_MS);
}