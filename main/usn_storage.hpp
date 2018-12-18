#ifndef USN_STORAGE_HPP
    #define USN_STORAGE_HPP

    #include <stdio.h>
    #include <dirent.h>
    #include <map>

    #define SPECIALVALUES_FILE "/spiffs/specialValues.json"  
    #define JSONBUFFERLEN 1024

    enum eSpecialValue {
        WIFI_SSID, 
        WIFI_PASS
    };

    const char sSpecialValue[][16] = {"WIFISSID", "WIFIPASS"}; 

    class cSPIFFSManager{
        private:
            static constexpr char *TAG = (char*)"SPIFFSManager";          
            
            static std::map<eSpecialValue, const char*> specialValues;
        public:
            cSPIFFSManager();
            static void _printMap();
            static void init();
            static bool fileExists(const char* filename);
            static DIR* getRootFolder();
            static FILE* getFile(const char* filename);

            static void getSpecialValue(eSpecialValue key, char* buffer);
            static void setSpecialValue(eSpecialValue key, char* buffer);
            static void setSpecialValue(char* key, char* buffer); 
            static void saveSpecialValue();
            static void restoreSpecialValue();

            static void test_writeWiFi_File();
    };

#endif