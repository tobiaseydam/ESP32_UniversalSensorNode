#ifndef USN_AWS_IOT
    #define USN_AWS_IOT

    #include "usn_storage.hpp"
    
    #include "aws_iot_config.h"
    #include "aws_iot_log.h"
    #include "aws_iot_version.h"
    #include "aws_iot_mqtt_client_interface.h"

    #define ROOT_CA_PEM_FILE    "/spiffs/root-ca-cert.pem"
    #define CERT_PEM_FILE       "/spiffs/cert.pem"
    #define PRIVATE_KEY_FILE    "/spiffs/private.key"

    class aws_iot_adapter{
        public:
            static constexpr char *TAG = (char*)"aws_iot_adapter16";
            static const char* aws_root_ca_pem; 
            static const char* aws_cert_pem; 
            static const char* aws_private_key_pem; 
            static storage_adapter* _sa;
            static void load_certs();
            static void set_storage_adapter(storage_adapter* sa);
            static void task(void *pvParams);
            static void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData);
    };

#endif