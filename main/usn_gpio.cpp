#include "usn_gpio.hpp"

void cGPIO::setPinMode(uint8_t pin, ePinMode mode){
    gpio_config_t io_conf;
    memset(&io_conf, 0, sizeof(io_conf));
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL<<pin);
    switch(mode){
        case INPUT:
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_down_en = (gpio_pulldown_t)0;
            io_conf.pull_up_en = (gpio_pullup_t)1;
            break;
        case OUTPUT:
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pull_down_en = (gpio_pulldown_t)1;
            io_conf.pull_up_en = (gpio_pullup_t)0;
            break;
    };
    gpio_config(&io_conf);
}


void cGPIO::setPin(uint8_t pin, uint32_t level){
    gpio_set_level((gpio_num_t)pin, level);
}

int cGPIO::getPin(uint8_t pin){
    return gpio_get_level((gpio_num_t)pin);
}