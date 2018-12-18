#include "usn_storage.hpp"

#include "esp_log.h"
#include "esp_spiffs.h"
#include <sys/stat.h>
#include <string.h>
#include "cJSON.h"      //  https://github.com/DaveGamble/cJSON/tree/7cc52f60356909b3dd260304c7c50c0693699353
#include <math.h>

std::map<eSpecialValue, const char*> cSPIFFSManager::specialValues;

cSPIFFSManager::cSPIFFSManager(){
    
}

void cSPIFFSManager::init(){
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);    

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    restoreSpecialValue();
}

bool cSPIFFSManager::fileExists(const char* filename){
    struct stat st;
    return (stat(filename, &st)==0);
}

DIR* cSPIFFSManager::getRootFolder(){
    return opendir("/spiffs");
}

FILE* cSPIFFSManager::getFile(const char* filename){
    if(fileExists(filename)){
        return fopen(filename, "r");
    }else{
        return NULL;
    }
}

void cSPIFFSManager::_printMap(){
    for (auto const& x : specialValues){
        ESP_LOGI(TAG, "%i - %s", x.first, x.second);
    }
}

void cSPIFFSManager::getSpecialValue(eSpecialValue key, char* buffer){
    strcpy(buffer, specialValues.at(key));
};
            
void cSPIFFSManager::setSpecialValue(eSpecialValue key, char* buffer){
    specialValues[key] = buffer;
    ESP_LOGI(TAG, "Value updated: %i = %s", key, specialValues[key]);
};
            
void cSPIFFSManager::setSpecialValue(char* key, char* buffer){
    for(uint8_t i = WIFI_SSID; i<=WIFI_PASS; i++){
        if(strcmp(sSpecialValue[i], key)==0){
            specialValues[eSpecialValue(i)] = buffer;
            ESP_LOGI(TAG, "Value updated: %s = %s", key, specialValues[eSpecialValue(i)]);
        }
    }
};

void cSPIFFSManager::saveSpecialValue(){
    cJSON *filecontent = cJSON_CreateObject();
    for(uint8_t i = WIFI_SSID; i<=WIFI_PASS; i++){
        cJSON_AddItemToObject(filecontent, sSpecialValue[i], 
            cJSON_CreateString(specialValues.at(eSpecialValue(i)))
        );
    }
    char *string = NULL;
    string = cJSON_Print(filecontent);
    ESP_LOGI(TAG, "JSON created: %s", string);
    cJSON_Delete(filecontent);

    FILE* f = fopen(SPECIALVALUES_FILE, "w");
    fprintf(f, string);
    fclose(f);
}

void cSPIFFSManager::restoreSpecialValue(){
    FILE* file = getFile(SPECIALVALUES_FILE);
    if(file){
        char* content = new char[JSONBUFFERLEN];
        struct stat st;
        stat(SPECIALVALUES_FILE, &st);
        fread(content, 1, st.st_size+1, file);
        content[st.st_size] = '\0';
        ESP_LOGI(TAG, "JSON restored: %s", content);
        cJSON* json = cJSON_Parse(content);

        for(uint8_t i = WIFI_SSID; i<=WIFI_PASS; i++){
            const cJSON *val = NULL;
            val = cJSON_GetObjectItemCaseSensitive(json, sSpecialValue[i]);
            if (cJSON_IsString(val) && (val->valuestring != NULL)){

                char* v = new char[strlen(val->valuestring)+1];
                memset(v, '\0', strlen(val->valuestring)+1); 
                strcpy(v, val->valuestring);

                specialValues.insert(std::pair<eSpecialValue, const char*>(eSpecialValue(i), v));
            }
        }

        cJSON_Delete(json);
        free(content);  
        fclose(file);     
        _printMap();
    }
}