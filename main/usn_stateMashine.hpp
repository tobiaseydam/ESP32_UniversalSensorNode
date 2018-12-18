#ifndef USN_STATEMASHINE_HPP
    #define USN_STATEMASHINE_HPP

    #include "usn_storage.hpp"
    #include "usn_wifi.hpp"

    #define WIFI_FILE       "/wifi.txt"
    #define MQTT_FILE       "/mqtt.txt"
    #define WIFI_TIMEOUT    5
    #define WIFI_AP_SSID    "ESP32"
    #define WIFI_AP_PASS    "MotDet"

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


        public:
            cStateMashine(cSPIFFSManager* spiffsManager, cWiFi* wifi);

            void begin(void *pvParameter);
            
    };

#endif