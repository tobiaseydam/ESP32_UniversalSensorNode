#include "usn_aws_iot.hpp"
#include "esp_log.h"
#include <string.h>

const char* aws_iot_adapter::aws_root_ca_pem = NULL;
const char* aws_iot_adapter::aws_cert_pem = NULL;
const char* aws_iot_adapter::aws_private_key_pem = NULL;
storage_adapter* aws_iot_adapter::_sa = NULL;

void aws_iot_adapter::load_certs(){
    if(_sa == NULL){
        return;
    }
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