#include "usn_aws_iot.hpp"
#include "esp_log.h"
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event_loop.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

const char* aws_iot_adapter::aws_root_ca_pem = NULL;
const char* aws_iot_adapter::aws_cert_pem = NULL;
const char* aws_iot_adapter::aws_private_key_pem = NULL;
storage_adapter* aws_iot_adapter::_sa = NULL;

void aws_iot_adapter::load_certs(){
    if(_sa == NULL){
        return;
    }
    //MBEDTLS_ERR_X509_CERT_VERIFY_FAILED               
    FILE* file = _sa->get_file(ROOT_CA_PEM_FILE);
    if(file){
        if(!aws_root_ca_pem){
            char *root_ca_pem = new char[_sa->get_file_size(ROOT_CA_PEM_FILE)]();
            char buffer[32];
            while(fgets(buffer, 32 , file)){
                strcat(root_ca_pem, buffer);
            }
            aws_root_ca_pem = root_ca_pem;
            ESP_LOGI("aws_iot", "%s", aws_root_ca_pem);
        }
    }
    file = _sa->get_file(CERT_PEM_FILE);
    if(file){
        if(!aws_cert_pem){
            char *cert_pem = new char[_sa->get_file_size(CERT_PEM_FILE)]();
            char buffer[32];
            while(fgets(buffer, 32 , file)){
                strcat(cert_pem, buffer);
            }
            aws_cert_pem = cert_pem;
            ESP_LOGI("aws_iot", "%s", aws_cert_pem);
        }
    }
    file = _sa->get_file(PRIVATE_KEY_FILE);
    if(file){
        if(!aws_private_key_pem){
            char *private_key_pem = new char[_sa->get_file_size(PRIVATE_KEY_FILE)]();
            char buffer[32];
            while(fgets(buffer, 32 , file)){
                strcat(private_key_pem, buffer);
            }
            aws_private_key_pem = private_key_pem;
            ESP_LOGI("aws_iot", "%s", aws_private_key_pem);
        }
    }
}

void aws_iot_adapter::set_storage_adapter(storage_adapter* sa){
    _sa = sa;
}

void aws_iot_adapter::task(void *pvParams){
    char cPayload[100];

    int32_t i = 0;
    IoT_Error_t rc = FAILURE;
    AWS_IoT_Client client;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    IoT_Publish_Message_Params paramsQOS0;
    IoT_Publish_Message_Params paramsQOS1;

    ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    //mqttInitParams.pHostURL = "192.168.178.43";
    mqttInitParams.pHostURL = "a38mp4h6o8iiol-ats.iot.us-east-1.amazonaws.com";
    mqttInitParams.port = 8883;

    mqttInitParams.pRootCALocation = aws_root_ca_pem;
    mqttInitParams.pDeviceCertLocation = aws_cert_pem;
    mqttInitParams.pDevicePrivateKeyLocation = aws_private_key_pem;

    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
        abort();
    }

    connectParams.keepAliveIntervalInSec = 10;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    /* Client ID is set in the menuconfig of the example */
    connectParams.pClientID = "1943708d0d";
    connectParams.clientIDLen = 10;
    connectParams.isWillMsgPresent = false;

    ESP_LOGI(TAG, "Connecting to AWS...");
    do {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if(SUCCESS != rc) {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while(SUCCESS != rc);

    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
        abort();
    }

    const char *TOPIC = "$aws/things/GG_WohnungBerlin_Core/shadow/get";
    const int TOPIC_LEN = strlen(TOPIC);

    ESP_LOGI(TAG, "Subscribing...");
    rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
        abort();
    }

    sprintf(cPayload, "%s : %d ", "hello from SDK", i);

    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void *) cPayload;
    paramsQOS0.isRetained = 0;

    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *) cPayload;
    paramsQOS1.isRetained = 0;

    while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

        //Max time the yield function will wait for read messages
        rc = aws_iot_mqtt_yield(&client, 100);
        if(NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
        }

        ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(1000 / portTICK_RATE_MS);
        sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS0)", i++);
        paramsQOS0.payloadLen = strlen(cPayload);
        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS0);

        sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS1)", i++);
        paramsQOS1.payloadLen = strlen(cPayload);
        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS1);
        if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
            ESP_LOGW(TAG, "QOS1 publish ack not received.");
            rc = SUCCESS;
        }
    }

    ESP_LOGE(TAG, "An error occurred in the main loop.");
    abort();
}

void aws_iot_adapter::iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    ESP_LOGI(TAG, "Subscribe callback");
    ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
}