#include "usn_http.hpp"
#include "esp_log.h"
#include "esp_err.h"
#include "usn_storage.hpp"
#include "esp_spiffs.h"
#include <stdio.h>
#include <dirent.h>

httpd_uri_t cHttp::uri_get;
httpd_uri_t cHttp::spiffs_get;


cHttp::cHttp(){
    memset(&uri_get, 0, sizeof(uri_get));  
    uri_get.uri      = "/uri";
    uri_get.method   = HTTP_GET;
    uri_get.handler  = get_handler;
    uri_get.user_ctx = NULL;   

    memset(&spiffs_get, 0, sizeof(spiffs_get));  
    spiffs_get.uri      = "/spiffs";
    spiffs_get.method   = HTTP_GET;
    spiffs_get.handler  = spiffs_get_handler;
    spiffs_get.user_ctx = NULL;   
}

paramset_t* cHttp::_parseURI(httpd_req_t *req){
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t param_start = 0;
    uint8_t param_end = 0;

    paramset_t* p = new paramset_t;
    memset(p, '\0', sizeof(*p));

    char *uri = new char[127];
    memset(uri, '\0', 127);

    uint8_t offs = 0;
    ESP_LOGI(TAG, "req: %s", req->uri);
    while(i<127){
        if(req->uri[i+offs]=='\0'){
            break;
        }
        if(strncmp(&(req->uri[i]),"%20",3)==0){
            uri[i] = ' ';
            offs += 2;
        }else{
            uri[i] = req->uri[i+offs];
        }
        i++;
    }
    ESP_LOGI(TAG, "req: %s", uri);
    i=0;
    while(i<127){
        if((uri[i] == '?')||(uri[i] == '&')||(uri[i]=='\0')){// new Param
            if(param_start>0){    // print old param
                strncpy(p->params[j], &(uri[param_start]), param_end-param_start);
                ESP_LOGI(TAG, "param: %s", p->params[j]);

                strncpy(p->vals[j], &(uri[param_end+1]), i-param_end-1);
                ESP_LOGI(TAG, "val: %s", p->vals[j]);

                j++;
            }
            param_start = i + 1;
        }
        if(uri[i] == '='){
            param_end = i;
        }
        if(uri[i] == '\0'){
            break;
        }
        i++;
    }
    free(uri);
    return p;
}


httpd_handle_t* cHttp::start_webserver(){
    ESP_LOGI(TAG, "http server started");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t* server = new httpd_handle_t;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(*server, &uri_get);
        httpd_register_uri_handler(*server, &spiffs_get);
    }
    return server;
}

esp_err_t cHttp::get_handler(httpd_req_t *req){
    const char resp[] = "URI GET Response";
    ESP_LOGI(TAG, "get request:");
    ESP_LOGI(TAG, "method: %i", req->method);
    ESP_LOGI(TAG, "uri: %s", req->uri);
    paramset_t* p = _parseURI(req);

    for(uint8_t i = 0; i<MAX_PARAMS; i++){
        /*if(strcmp(p->params[i], "WIFISSID")==0){
            cSPIFFSManager::setSpecialValue(WIFI_SSID, p->vals[i]);
        }
        if(strcmp(p->params[i], "WIFIPASS")==0){
            cSPIFFSManager::setSpecialValue(WIFI_PASS, p->vals[i]);
        }*/
        cSPIFFSManager::setSpecialValue(p->params[i], p->vals[i]);

    }
    cSPIFFSManager::saveSpecialValue();

    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t cHttp::spiffs_get_handler(httpd_req_t *req){
    char* resp = new char[127];
    
    paramset_t* p = _parseURI(req);
    if(strcmp(p->params[0], "file")==0){
        if((strcmp(p->params[1], "action")==0)&&(strcmp(p->vals[1], "open")==0)){
            FILE* file = cSPIFFSManager::getFile(p->vals[0]);
            if(file){
                httpd_resp_set_type(req, "text");

                sprintf(resp, "File: %s \n\n", p->vals[0]);
                httpd_resp_send_chunk(req, resp, strlen(resp));

                while(fgets(resp, 127, file)){
                    httpd_resp_send_chunk(req, resp, strlen(resp));
                }

                httpd_resp_send_chunk(req, NULL, 0);
                fclose(file);
            }else{
                sprintf(resp, "File not found: %s", p->vals[0]);
                httpd_resp_send(req, resp, strlen(resp));
            }
        }else if((strcmp(p->params[1], "action")==0)&&(strcmp(p->vals[1], "delete")==0)){
            FILE* file = cSPIFFSManager::getFile(p->vals[0]);
            if(file){
                fclose(file);
                remove(p->vals[0]);
                sprintf(resp, "File deleted: %s<br><a href='/spiffs'>spiffs</a>", p->vals[0]);
                httpd_resp_send(req, resp, strlen(resp));
            }else{
                sprintf(resp, "File not found: %s", p->vals[0]);
                httpd_resp_send(req, resp, strlen(resp));
            }
        }
    }else{
        strcpy(resp, "spiffs filesystem: <br>");
        httpd_resp_send_chunk(req, resp, strlen(resp));

        size_t total = 0, used = 0;
        ESP_ERROR_CHECK(esp_spiffs_info(NULL, &total, &used));
        sprintf(resp, "Space: %d Bytes, used: %d Bytes<br><br>", total, used);
        httpd_resp_send_chunk(req, resp, strlen(resp));

        sprintf(resp, "<table><tr><th>file</th><th>action</th></tr>");
        httpd_resp_send_chunk(req, resp, strlen(resp));

        DIR* root = cSPIFFSManager::getRootFolder();
        struct dirent *ent;
        while ((ent = readdir(root)) != NULL) {
            sprintf(resp, "<tr><td>");
            httpd_resp_send_chunk(req, resp, strlen(resp));

            sprintf(resp, "<a href='?file=/spiffs/%s&action=open'>%s</a>", ent->d_name, ent->d_name);
            httpd_resp_send_chunk(req, resp, strlen(resp));
            
            sprintf(resp, "</td><td>");
            httpd_resp_send_chunk(req, resp, strlen(resp));

            sprintf(resp, "<a href='?file=/spiffs/%s&action=delete'>delete</a>", ent->d_name);
            httpd_resp_send_chunk(req, resp, strlen(resp));
            
            sprintf(resp, "</td></tr>");
            httpd_resp_send_chunk(req, resp, strlen(resp));
        }
        closedir(root);

        sprintf(resp, "</table>");
        httpd_resp_send_chunk(req, resp, strlen(resp));
        
        httpd_resp_send_chunk(req, NULL, 0);
    }
    free(resp);
    return ESP_OK;
}

void cHttp::stop_webserver(httpd_handle_t server){
    if (server) {
        httpd_stop(server);
    }
}

void cHttp::dummy(){
    
}
