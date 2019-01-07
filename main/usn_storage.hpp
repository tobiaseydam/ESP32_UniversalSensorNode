#ifndef USN_STORAGE_HPP
    #define USN_STORAGE_HPP

    #include <stdio.h>
    #include <dirent.h>
    #include <map>

    #include "usn_lcd.hpp"

    #define GLOBALS_FILE "/spiffs/globals.json"
    #define JSON_BUFFER_LEN 1024

    enum e_global_value {
        WIFISSID, 
        WIFIPASS,
        _LAST
    };

    const char s_global_value[][16] = {"WIFISSID", "WIFIPASS", "_LAST"};

    class storage_adapter{
        private:
            static constexpr char *TAG = (char*)"storage_adapter";    
            display_buffer_t* _db;

            std::map<e_global_value, const char*> global_values;
            void load_global_values();
        public:
            storage_adapter();
            void set_display_buffer(display_buffer_t* db);

            void _print_map();
            void init();

            bool file_exists(const char* filename);
            DIR* get_root_folder();
            FILE* get_file(const char* filename);
            long get_file_size(const char* filename);
            const char* get_global_value(e_global_value key);
            void set_global_value(e_global_value key, const char* value);
            void set_global_value(const char* key, const char* value);
            void save_global_values();
    };

#endif