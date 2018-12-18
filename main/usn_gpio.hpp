#ifndef USN_GPIO_HPP
    #define USN_GPIO_HPP

    #include <stdint.h>
    #include <string.h>
    #include "driver/gpio.h"

    enum ePinMode{
        INPUT,
        OUTPUT
    };

    class cGPIO{
        public:
            static void setPinMode(uint8_t pin, ePinMode mode);
            static void setPin(uint8_t pin, uint32_t level);
            static int getPin(uint8_t pin);
    };

#endif