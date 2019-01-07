#include "usn_storage.hpp"

#include "esp_log.h"
#include "esp_spiffs.h"
#include <sys/stat.h>
#include <string.h>
#include "cJSON.h"      //  https://github.com/DaveGamble/cJSON/tree/7cc52f60356909b3dd260304c7c50c0693699353
#include <math.h>

//----- storage_adapter ------------------------------------------------

storage_adapter::storage_adapter(){

}

void storage_adapter::init(){
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

    load_global_values();
}

void storage_adapter::set_display_buffer(display_buffer_t* db){
    _db = db;
}

bool storage_adapter::file_exists(const char* filename){
    struct stat st;
    return (stat(filename, &st)==0);
}

long storage_adapter::get_file_size(const char* filename){
    struct stat st;
    stat(filename, &st);
    ESP_LOGI(TAG, "File %s - size: %ld Bytes", filename, st.st_size);
    return st.st_size;
}

DIR* storage_adapter::get_root_folder(){
    return opendir("/spiffs");
}

FILE* storage_adapter::get_file(const char* filename){
    if(file_exists(filename)){
        return fopen(filename, "r");
    }else{
        return NULL;
    }
}

void storage_adapter::_print_map(){
    for (auto const& x : global_values){
        ESP_LOGI(TAG, "%i - %s : %s", x.first, s_global_value[x.first] , x.second);
    }
}

void storage_adapter::set_global_value(e_global_value key, const char* value){
    global_values[key] = value;
    ESP_LOGI(TAG, "Value updated: %s = %s", s_global_value[key], global_values[key]);
}

void storage_adapter::set_global_value(const char* key, const char* value){
    for(uint8_t i = WIFISSID; i<=_LAST; i++){
        if(strcmp(s_global_value[i], key)==0){
            global_values[e_global_value(i)] = value;
            ESP_LOGI(TAG, "Value updated: %s = %s", key, global_values[e_global_value(i)]);
        }
    }
}

const char* storage_adapter::get_global_value(e_global_value key){
    return global_values.at(key);
}

void storage_adapter::save_global_values(){
    cJSON *filecontent = cJSON_CreateObject();
    for (auto const& x : global_values){
        cJSON_AddItemToObject(filecontent, 
            s_global_value[x.first], 
            cJSON_CreateString(x.second)
        );
    }
    char *string = NULL;
    string = cJSON_Print(filecontent);
    ESP_LOGI(TAG, "JSON created: %s", string);
    cJSON_Delete(filecontent);

    FILE* f = fopen(GLOBALS_FILE, "w");
    fprintf(f, string);
    fclose(f);
}
            
void storage_adapter::load_global_values(){
    FILE* file = get_file(GLOBALS_FILE);
    if(file){
        char* content = new char[JSON_BUFFER_LEN];
        struct stat st;
        stat(GLOBALS_FILE, &st);
        fread(content, 1, st.st_size+1, file);
        content[st.st_size] = '\0';
        ESP_LOGI(TAG, "JSON restored: %s", content);
        cJSON* json = cJSON_Parse(content);

        for(uint8_t i = WIFISSID; i<=_LAST; i++){
            const cJSON *val = NULL;
            val = cJSON_GetObjectItemCaseSensitive(json, s_global_value[i]);
            if (cJSON_IsString(val) && (val->valuestring != NULL)){

                char* v = new char[strlen(val->valuestring)+1];
                memset(v, '\0', strlen(val->valuestring)+1); 
                strcpy(v, val->valuestring);

                global_values.insert(std::pair<e_global_value, const char*>(e_global_value(i), v));
            }
        }

        cJSON_Delete(json);
        free(content);  
        fclose(file);     
        _print_map();
    }
}