#include "usn_http.hpp"
#include "esp_log.h"
#include "esp_err.h"
#include "usn_storage.hpp"
#include "esp_spiffs.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "esp_event_loop.h"
#include "esp_system.h"
#include "sys/param.h"

httpd_uri_t http_server::root_get;
httpd_uri_t http_server::spiffs_get;
httpd_uri_t http_server::upload_post;
storage_adapter* http_server::_sa = NULL;

http_server::http_server(){
    memset(&root_get, 0, sizeof(root_get));  
    root_get.uri      = "/";
    root_get.method   = HTTP_GET;
    root_get.handler  = get_handler;
    root_get.user_ctx = NULL;

    memset(&spiffs_get, 0, sizeof(spiffs_get));  
    spiffs_get.uri      = "/spiffs";
    spiffs_get.method   = HTTP_GET;
    spiffs_get.handler  = spiffs_get_handler;
    spiffs_get.user_ctx = NULL;

    memset(&upload_post, 0, sizeof(upload_post));  
    upload_post.uri      = "/upload";
    upload_post.method   = HTTP_POST;
    upload_post.handler  = upload_post_handler;
    upload_post.user_ctx = NULL;
}

paramset_t* http_server::_parseURI(httpd_req_t *req){
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
    p->num_sets = j;
    free(uri);
    return p;
}


httpd_handle_t* http_server::start_webserver(){
    ESP_LOGI(TAG, "http server started");
    esp_err_t ret = ESP_OK;
    httpd_handle_t* server = new httpd_handle_t;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ret = httpd_start(server, &config);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(*server, &root_get);
        httpd_register_uri_handler(*server, &spiffs_get);
        httpd_register_uri_handler(*server, &upload_post);
    }
    return server;
}

esp_err_t http_server::get_handler(httpd_req_t *req){
    const char resp[] = "GET Response";
    ESP_LOGI(TAG, "get request:");
    ESP_LOGI(TAG, "method: %i", req->method);
    ESP_LOGI(TAG, "uri: %s", req->uri);
    paramset_t* p = _parseURI(req);

    if(p->num_sets>0){
        for(uint8_t i = 0; i<MAX_PARAMS; i++){
            _sa->set_global_value(p->params[i], p->vals[i]);
        }
        _sa->save_global_values();
    }
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t http_server::spiffs_get_handler(httpd_req_t *req){
    char* resp = new char[127];
    
    paramset_t* p = _parseURI(req);
    if(strcmp(p->params[0], "file")==0){
        if((strcmp(p->params[1], "action")==0)&&(strcmp(p->vals[1], "open")==0)){
            FILE* file = _sa->get_file(p->vals[0]);
            if(file){
                char *ext = strrchr(p->vals[0], '.');
                ESP_LOGI("http", "ext: %s", ext);

                if((strcmp(ext, ".htm") == 0)||(strcmp(ext, ".html") == 0)){
                    httpd_resp_set_type(req, "text/html");
                }else{
                    httpd_resp_set_type(req, "text");
                    sprintf(resp, "File: %s \n\n", p->vals[0]);
                    httpd_resp_send_chunk(req, resp, strlen(resp));
                }

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
            FILE* file = _sa->get_file(p->vals[0]);
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

        DIR* root = _sa->get_root_folder();
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

esp_err_t http_server::upload_post_handler(httpd_req_t *req){
    char buffer[100];
    int remaining = req->content_len;
    
    const char* resp = "URI POST Response";

    ESP_LOGI("http", "content_len: %d", remaining);
    uint8_t j = 0;
    uint8_t k = 0;
    uint8_t last_header_line = 255;
    char first_line[256];
    char line[256];
    FILE* f = NULL;

    esp_err_t ret;
    while (remaining > 0) {
        /* Read the data for the request */
        
        if ((ret = httpd_req_recv(req, buffer, MIN(remaining, sizeof(buffer)))) < 0) {
            return ESP_FAIL;
        }

        remaining -= ret;

        for(uint8_t i = 0; i<ret; i++){
            if(buffer[i]!='\n'){
                line[j] = buffer[i];
                j++;
            }else{
                line[j] = '\0';
                if(k==0){
                    strcpy(first_line, line);
                    ESP_LOGI("http", "first: %s", first_line);
                }
                k++;
                if(strncmp(line, "Content-Disposition: ", 20)==0){
                    char name[32];
                    char filename[32];
                    uint8_t p1 = strstr(line, "name=\"") - line;
                    uint8_t p2 = strstr(&line[p1], "\";") - line;
                    if((p1>0)&&(p2>0)){
                        strncpy(name, &line[p1+6], p2-p1-6);
                        name[p2-p1-6] = '\0';
                        ESP_LOGI("http", "name: %s", name);
                    }
                    uint8_t p3 = strstr(&line[p2], "filename=\"") - line;
                    uint8_t p4 = strstr(&line[p3+10], "\"") - line;
                    if((p3>0)&&(p4>0)){
                        strncpy(filename, &line[p3+10], p4-p3-10);
                        filename[p4-p3-10] = '\0';
                        ESP_LOGI("http", "filename: %s", filename);
                    }

                    if(strcmp(name, "uploadfile")==0){
                        last_header_line = k + 2;
                        
                        char spiffs_filename[64];
                        strcat(spiffs_filename, "/spiffs/");
                        strcat(spiffs_filename, filename);
                        ESP_LOGI("http", "saving file: %s", spiffs_filename);
                        f = fopen(spiffs_filename, "w");
                    }
                }

                if(k > last_header_line){
                    if(strncmp(first_line, line, strlen(first_line)-1)!=0){
                        ESP_LOGI("http", "Line (%d): %s", k, line);
                        if(f){
                            fprintf(f, line);
                            fprintf(f, "\n");
                        }
                    }else{
                        if(f){
                            fclose(f);
                            ESP_LOGI("http", "File written");
                            resp = "File uploaded";
                        }
                    }
                }
                
                j = 0;
            }
        }
        //Content-Disposition: form-data; name="file1"; filename="upload.html"
        //ESP_LOGI("http", "%s", buffer);
    }

    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

void http_server::stop_webserver(httpd_handle_t server){
    if (server) {
        httpd_stop(server);
    }
}

void http_server::set_storage_adapter(storage_adapter* sa){
    http_server::_sa = sa;
}
