#include "usn_lcd.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <stdio.h>
// Erzeugt einen Enable-Puls
void cLCD::lcd_enable(){
    //LCD_PORT |= (1<<LCD_EN);     // Enable auf 1 setzen
    cGPIO::setPin(LCD_EN, 1);
    //_delay_us( LCD_ENABLE_US );  // kurze Pause
    vTaskDelay( LCD_ENABLE_US / portTICK_PERIOD_MS);
    //LCD_PORT &= ~(1<<LCD_EN);    // Enable auf 0 setzen
    cGPIO::setPin(LCD_EN, 0);
}

// Sendet eine 4-bit Ausgabeoperation an das LCD
void cLCD::lcd_out(uint8_t data){
    data &= 0xF0;                       // obere 4 Bit maskieren
    ESP_LOGI("LCD", "data: %d", data);
    uint8_t d7 = (data & 0x80)>>7;
    uint8_t d6 = (data & 0x40)>>6;
    uint8_t d5 = (data & 0x20)>>5;
    uint8_t d4 = (data & 0x10)>>4;
    ESP_LOGI("LCD", "d7: %d", d7);
    ESP_LOGI("LCD", "d6: %d", d6);
    ESP_LOGI("LCD", "d5: %d", d5);
    ESP_LOGI("LCD", "d4: %d", d4);
    

    //LCD_PORT &= ~(0xF0>>(4-LCD_DB));    // Maske löschen
    cGPIO::setPin(LCD_DB7,0);
    cGPIO::setPin(LCD_DB6,0);
    cGPIO::setPin(LCD_DB5,0);
    cGPIO::setPin(LCD_DB4,0);

    //LCD_PORT |= (data>>(4-LCD_DB));     // Bits setzen
    cGPIO::setPin(LCD_DB7,d7);
    cGPIO::setPin(LCD_DB6,d6);
    cGPIO::setPin(LCD_DB5,d5);
    cGPIO::setPin(LCD_DB4,d4);

    cLCD::lcd_enable();
}

void cLCD::lcd_init(){
    // verwendete Pins auf Ausgang schalten
    //uint8_t pins = (0x0F << LCD_DB) |           // 4 Datenleitungen
    //               (1<<LCD_RS) |                // R/S Leitung
    //               (1<<LCD_EN);                 // Enable Leitung
    //LCD_DDR |= pins;
    cGPIO::setPinMode(LCD_DB4, OUTPUT);
    cGPIO::setPinMode(LCD_DB5, OUTPUT);
    cGPIO::setPinMode(LCD_DB6, OUTPUT);
    cGPIO::setPinMode(LCD_DB7, OUTPUT);
    cGPIO::setPinMode(LCD_EN, OUTPUT);
    cGPIO::setPinMode(LCD_RS, OUTPUT);
    cGPIO::setPinMode(LCD_RW, OUTPUT);

    // initial alle Ausgänge auf Null
    //LCD_PORT &= ~pins;
    cGPIO::setPin(LCD_DB4,0);
    cGPIO::setPin(LCD_DB5,0);
    cGPIO::setPin(LCD_DB6,0);
    cGPIO::setPin(LCD_DB7,0);
    cGPIO::setPin(LCD_EN,0);
    cGPIO::setPin(LCD_RS,0);
    cGPIO::setPin(LCD_RW,0);

    // warten auf die Bereitschaft des LCD
    vTaskDelay( LCD_BOOTUP_MS / portTICK_PERIOD_MS);
    
    // Soft-Reset muss 3mal hintereinander gesendet werden zur Initialisierung
    cLCD::lcd_out( LCD_SOFT_RESET );
    vTaskDelay( LCD_SOFT_RESET_MS1 / portTICK_PERIOD_MS);
 
    cLCD::lcd_enable();
    vTaskDelay( LCD_SOFT_RESET_MS2 / portTICK_PERIOD_MS);
 
    cLCD::lcd_enable();
    vTaskDelay( LCD_SOFT_RESET_MS3 / portTICK_PERIOD_MS);
 
    // 4-bit Modus aktivieren 
    cLCD::lcd_out( LCD_SET_FUNCTION |
             LCD_FUNCTION_4BIT );
    vTaskDelay( LCD_SET_4BITMODE_MS / portTICK_PERIOD_MS);
 
    // 4-bit Modus / 2 Zeilen / 5x7
    cLCD::lcd_command( LCD_SET_FUNCTION |
                 LCD_FUNCTION_4BIT |
                 LCD_FUNCTION_2LINE |
                 LCD_FUNCTION_5X7 );
 
    // Display ein / Cursor aus / Blinken aus
    cLCD::lcd_command( LCD_SET_DISPLAY |
                 LCD_DISPLAY_ON |
                 LCD_CURSOR_OFF |
                 LCD_BLINKING_OFF); 
 
    // Cursor inkrement / kein Scrollen
    cLCD::lcd_command( LCD_SET_ENTRY |
                 LCD_ENTRY_INCREASE |
                 LCD_ENTRY_NOSHIFT );
 
    cLCD::lcd_clear();
}

// Sendet ein Datenbyte an das LCD
void cLCD::lcd_data( uint8_t data ){
    //LCD_PORT |= (1<<LCD_RS);    // RS auf 1 setzen
    cGPIO::setPin(LCD_RS,1);

    cLCD::lcd_out( data );            // zuerst die oberen, 
    cLCD::lcd_out( data<<4 );         // dann die unteren 4 Bit senden
 
    vTaskDelay( LCD_WRITEDATA_US / portTICK_PERIOD_MS);
}

// Sendet einen Befehl an das LCD
void cLCD::lcd_command( uint8_t data ){
    //LCD_PORT &= ~(1<<LCD_RS);    // RS auf 0 setzen
    cGPIO::setPin(LCD_RS,0);

    cLCD::lcd_out( data );             // zuerst die oberen, 
    cLCD::lcd_out( data<<4 );           // dann die unteren 4 Bit senden
 
    vTaskDelay( LCD_COMMAND_US / portTICK_PERIOD_MS);
}

// Sendet den Befehl zur Löschung des Displays
void cLCD::lcd_clear( void ){
    cLCD::lcd_command( LCD_CLEAR_DISPLAY );
    vTaskDelay( LCD_CLEAR_DISPLAY_MS / portTICK_PERIOD_MS);
}

// Sendet den Befehl: Cursor Home
void cLCD::lcd_home( void ){
    cLCD::lcd_command( LCD_CURSOR_HOME );
    vTaskDelay( LCD_CURSOR_HOME_MS / portTICK_PERIOD_MS);
}

// Setzt den Cursor in Spalte x (0..15) Zeile y (1..4) 
void cLCD::lcd_setcursor( uint8_t x, uint8_t y ){
    uint8_t data;
 
    switch (y)
    {
        case 1:    // 1. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE1 + x;
            break;
 
        case 2:    // 2. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE2 + x;
            break;
 
        case 3:    // 3. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE3 + x;
            break;
 
        case 4:    // 4. Zeile
            data = LCD_SET_DDADR + LCD_DDADR_LINE4 + x;
            break;
 
        default:
            return;                                   // für den Fall einer falschen Zeile
    }
 
    cLCD::lcd_command( data );
}

void cLCD::lcd_string( const char *data ){
    while( *data != '\0' )
        cLCD::lcd_data( *data++ );
}


