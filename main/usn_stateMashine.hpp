#ifndef USN_STATEMASHINE_HPP
    #define USN_STATEMASHINE_HPP

    #include "usn_storage.hpp"
    #include "usn_wifi.hpp"
    #include "usn_gpio.hpp"
    #include <stdio.h>

    #define WIFI_FILE       "/wifi.txt"
    #define MQTT_FILE       "/mqtt.txt"
    #define WIFI_TIMEOUT    5
    #define WIFI_AP_SSID    "ESP32"
    #define WIFI_AP_PASS    "MotDet"

    #define BTN1            21
    #define BTN2            19
    #define BTN3            18
    #define BTN4             5
    #define BTN5            17
    #define BTN6            16
    #define BTN7             4

    class cStateMashine{
        private:
            const char *TAG = "StateMashine";

            cSPIFFSManager* _spiffs;
            cWiFi* _wifi;

            enum eSMState {
                START, 

                WIFI_LOOK_FOR_DATA, 
                WIFI_LOGIN,
                /*WIFI_OPEN_ACCESSPOINT,
                WIFI_WAIT_FOR_CONFIG,

                MQTT_LOOK_FOR_DATA,
                MQTT_LOGIN,

                MAIN_HANDLE_MQTT,
                */
                UNRESOLVABLE_ERROR
            };

            enum eSMRunningState {
                NOT_STARTED, 
                RUNNING, 
                PAUSED, 
                ERROR
            };

            eSMState _state = START;
            eSMRunningState _runningState = NOT_STARTED;
        
            void _nextStep();
            void _start();
            void _wifiLookForData();
            void _wifiLogin();
            void _unresolvableError();

            static bool btn_val[7];
            static uint8_t btn_pin[7];

            static void _readBtn(void *pvParameters);

        public:
            cStateMashine(cSPIFFSManager* spiffsManager, cWiFi* wifi);

            void begin(void *pvParameter);
            
    };

#endif