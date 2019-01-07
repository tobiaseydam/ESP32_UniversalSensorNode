#ifndef USN_AWS_IOT
    #define USN_AWS_IOT

    #include "usn_storage.hpp"

    #define ROOT_CA_PEM_FILE    "/spiffs/root-ca-cert.pem"
    #define CERT_PEM_FILE       "/spiffs/cert.pem"
    #define PRIVATE_KEY_FILE    "/spiffs/private.key"

    class aws_iot_adapter{
        public:
            static const char* aws_root_ca_pem; 
            static const char* aws_cert_pem; 
            static const char* aws_private_key_pem; 
            static storage_adapter* _sa;
            static void load_certs();
            static void set_storage_adapter(storage_adapter* sa);
    };

#endif