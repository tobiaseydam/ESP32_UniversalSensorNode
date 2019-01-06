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
#include <algorithm>


//----- display_t ----------------------------------------------

// Erzeugt einen Enable-Puls
void display_t::lcd_enable(){
    //LCD_PORT |= (1<<LCD_EN);     // Enable auf 1 setzen
    cGPIO::setPin(LCD_EN, 1);
    //_delay_us( LCD_ENABLE_US );  // kurze Pause
    vTaskDelay( LCD_ENABLE_US / portTICK_PERIOD_MS);
    //LCD_PORT &= ~(1<<LCD_EN);    // Enable auf 0 setzen
    cGPIO::setPin(LCD_EN, 0);
}

// Sendet eine 4-bit Ausgabeoperation an das LCD
void display_t::lcd_out(uint8_t data){
    data &= 0xF0;                       // obere 4 Bit maskieren
    uint8_t d7 = (data & 0x80)>>7;
    uint8_t d6 = (data & 0x40)>>6;
    uint8_t d5 = (data & 0x20)>>5;
    uint8_t d4 = (data & 0x10)>>4;
    
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

    display_t::lcd_enable();
}

void display_t::lcd_init(){
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
    display_t::lcd_out( LCD_SOFT_RESET );
    vTaskDelay( LCD_SOFT_RESET_MS1 / portTICK_PERIOD_MS);
 
    display_t::lcd_enable();
    vTaskDelay( LCD_SOFT_RESET_MS2 / portTICK_PERIOD_MS);
 
    display_t::lcd_enable();
    vTaskDelay( LCD_SOFT_RESET_MS3 / portTICK_PERIOD_MS);
 
    // 4-bit Modus aktivieren 
    display_t::lcd_out( LCD_SET_FUNCTION |
             LCD_FUNCTION_4BIT );
    vTaskDelay( LCD_SET_4BITMODE_MS / portTICK_PERIOD_MS);
 
    // 4-bit Modus / 2 Zeilen / 5x7
    display_t::lcd_command( LCD_SET_FUNCTION |
                 LCD_FUNCTION_4BIT |
                 LCD_FUNCTION_2LINE |
                 LCD_FUNCTION_5X7 );
 
    // Display ein / Cursor aus / Blinken aus
    display_t::lcd_command( LCD_SET_DISPLAY |
                 LCD_DISPLAY_ON |
                 LCD_CURSOR_OFF |
                 LCD_BLINKING_OFF); 
 
    // Cursor inkrement / kein Scrollen
    display_t::lcd_command( LCD_SET_ENTRY |
                 LCD_ENTRY_INCREASE |
                 LCD_ENTRY_NOSHIFT );
 
    display_t::lcd_clear();
}

// Sendet ein Datenbyte an das LCD
void display_t::lcd_data( uint8_t data ){
    //LCD_PORT |= (1<<LCD_RS);    // RS auf 1 setzen
    cGPIO::setPin(LCD_RS,1);

    display_t::lcd_out( data );            // zuerst die oberen, 
    display_t::lcd_out( data<<4 );         // dann die unteren 4 Bit senden
 
    vTaskDelay( LCD_WRITEDATA_US / portTICK_PERIOD_MS);
}

// Sendet einen Befehl an das LCD
void display_t::lcd_command( uint8_t data ){
    //LCD_PORT &= ~(1<<LCD_RS);    // RS auf 0 setzen
    cGPIO::setPin(LCD_RS,0);

    display_t::lcd_out( data );             // zuerst die oberen, 
    display_t::lcd_out( data<<4 );           // dann die unteren 4 Bit senden
 
    vTaskDelay( LCD_COMMAND_US / portTICK_PERIOD_MS);
}

// Sendet den Befehl zur Löschung des Displays
void display_t::lcd_clear( void ){
    display_t::lcd_command( LCD_CLEAR_DISPLAY );
    vTaskDelay( LCD_CLEAR_DISPLAY_MS / portTICK_PERIOD_MS);
}

// Sendet den Befehl: Cursor Home
void display_t::lcd_home( void ){
    display_t::lcd_command( LCD_CURSOR_HOME );
    vTaskDelay( LCD_CURSOR_HOME_MS / portTICK_PERIOD_MS);
}

// Setzt den Cursor in Spalte x (0..15) Zeile y (1..4) 
void display_t::lcd_setcursor( uint8_t x, uint8_t y ){
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
 
    display_t::lcd_command( data );
}

void display_t::lcd_string( const char *data ){
    while( *data != '\0' )
        display_t::lcd_data( *data++ );
}


void display_t::run(void* pvParameter){
    display_buffer_t* db = (display_buffer_t*)pvParameter;
    while(true){
        if(!(db->empty())){
            display_message_t* dm = db->dequeue();
            display_t::lcd_clear();
            display_t::lcd_setcursor(0,1);
            display_t::lcd_string(dm->get_line1());
            display_t::lcd_setcursor(0,2);
            display_t::lcd_string(dm->get_line2());
            //delete(db);
        }
    }
}

//----- display_message_t --------------------------------------

display_message_t::display_message_t(){
    _init();
}

display_message_t::display_message_t(const char* l1, const char* l2){
    _init();
    strncpy(_line1, l1, strlen(l1));
    strncpy(_line2, l2, strlen(l2));
}

void display_message_t::_init(){
    memset(&_line1, '\0', 16);
    memset(&_line2, '\0', 16);
}

/*
void display_message_t::set_line1(const char* format, ...){

}

void display_message_t::set_line2(const char* format, ...){

}
*/

void display_message_t::printToSerial(){
    ESP_LOGI("display_message", "MESSAGE: ");
    ESP_LOGI("display_message", "%s", _line1);
    ESP_LOGI("display_message", "%s", _line2);
}

const char* display_message_t::get_line1(){
    return _line1;
}

const char* display_message_t::get_line2(){
    return _line2;
}

//----- display_buffer_t ---------------------------------------

display_buffer_t::display_buffer_t(){
    ESP_LOGD(TAG, "init queue");
    _buffer = xQueueCreate(10, sizeof(display_message_t*));
}

void display_buffer_t::enqueue(display_message_t* message){
    ESP_LOGD(TAG, "add to queue");
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendToBackFromISR(_buffer, &message, &xHigherPriorityTaskWoken);
    _len += 1;
}

display_message_t* display_buffer_t::dequeue(){
    display_message_t* message;
    BaseType_t xTaskWokenByReceive = pdFALSE;
    xQueueReceiveFromISR(_buffer, &message, &xTaskWokenByReceive);
    _len -= 1;
    return message;
}

bool display_buffer_t::empty(){
    return _len==0;
}



