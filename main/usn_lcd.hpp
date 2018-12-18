#ifndef USN_LCD_HPP
    #define USN_LCD_HPP
    #include "usn_gpio.hpp"

    #define LCD_EN                  26
    #define LCD_RS                  14
    #define LCD_RW                  27
    #define LCD_DB4                 33
    #define LCD_DB5                 32   
    #define LCD_DB6                 22
    #define LCD_DB7                 23

    #define LCD_BOOTUP_MS           15
    #define LCD_ENABLE_US           0.02   //20
    #define LCD_WRITEDATA_US        0.046  //46
    #define LCD_COMMAND_US          0.042  //42

 
    #define LCD_SOFT_RESET_MS1      5
    #define LCD_SOFT_RESET_MS2      1
    #define LCD_SOFT_RESET_MS3      1
    #define LCD_SET_4BITMODE_MS     5

    #define LCD_CLEAR_DISPLAY_MS    2
    #define LCD_CURSOR_HOME_MS      2

    #define LCD_DDADR_LINE1         0x00
    #define LCD_DDADR_LINE2         0x40
    #define LCD_DDADR_LINE3         0x10
    #define LCD_DDADR_LINE4         0x50

    ////////////////////////////////////////////////////////////////////////////////
    // LCD Befehle und Argumente.
    // Zur Verwendung in lcd_command
    
    // Clear Display -------------- 0b00000001
    #define LCD_CLEAR_DISPLAY       0x01
    
    // Cursor Home ---------------- 0b0000001x
    #define LCD_CURSOR_HOME         0x02
    
    // Set Entry Mode ------------- 0b000001xx
    #define LCD_SET_ENTRY           0x04
    
    #define LCD_ENTRY_DECREASE      0x00
    #define LCD_ENTRY_INCREASE      0x02
    #define LCD_ENTRY_NOSHIFT       0x00
    #define LCD_ENTRY_SHIFT         0x01
    
    // Set Display ---------------- 0b00001xxx
    #define LCD_SET_DISPLAY         0x08
    
    #define LCD_DISPLAY_OFF         0x00
    #define LCD_DISPLAY_ON          0x04
    #define LCD_CURSOR_OFF          0x00
    #define LCD_CURSOR_ON           0x02
    #define LCD_BLINKING_OFF        0x00
    #define LCD_BLINKING_ON         0x01
    
    // Set Shift ------------------ 0b0001xxxx
    #define LCD_SET_SHIFT           0x10
    
    #define LCD_CURSOR_MOVE         0x00
    #define LCD_DISPLAY_SHIFT       0x08
    #define LCD_SHIFT_LEFT          0x00
    #define LCD_SHIFT_RIGHT         0x04
    
    // Set Function --------------- 0b001xxxxx
    #define LCD_SET_FUNCTION        0x20
    
    #define LCD_FUNCTION_4BIT       0x00
    #define LCD_FUNCTION_8BIT       0x10
    #define LCD_FUNCTION_1LINE      0x00
    #define LCD_FUNCTION_2LINE      0x08
    #define LCD_FUNCTION_5X7        0x00
    #define LCD_FUNCTION_5X10       0x04
    
    #define LCD_SOFT_RESET          0x30
    
    // Set CG RAM Address --------- 0b01xxxxxx  (Character Generator RAM)
    #define LCD_SET_CGADR           0x40
    
    #define LCD_GC_CHAR0            0
    #define LCD_GC_CHAR1            1
    #define LCD_GC_CHAR2            2
    #define LCD_GC_CHAR3            3
    #define LCD_GC_CHAR4            4
    #define LCD_GC_CHAR5            5
    #define LCD_GC_CHAR6            6
    #define LCD_GC_CHAR7            7
    
    // Set DD RAM Address --------- 0b1xxxxxxx  (Display Data RAM)
    #define LCD_SET_DDADR           0x80
    
    //from: https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/LCD-Ansteuerung
    class cLCD{
        public: 
            static void lcd_enable();
            static void lcd_out(uint8_t data);
            static void lcd_init();
            static void lcd_data( uint8_t data );
            static void lcd_command( uint8_t data );
            static void lcd_clear( void );
            static void lcd_home( void );
            static void lcd_setcursor( uint8_t x, uint8_t y );
            static void lcd_string( const char *data );
    };

#endif