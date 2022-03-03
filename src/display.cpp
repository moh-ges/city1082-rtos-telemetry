/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include <ios>
#include <iostream>
#include "string.h"
#include <iomanip>
#include "display.h"
#include "vt100.h"

void updateDisplay();
typedef struct {
    int     type;           /* The type of data 0 = Temperature   */
                            /*                 10 = Light Level   */
                            /*                 20 = ToDo          */
    float   value;          /* AD result of measured temperature  */
} message_t;
typedef struct {
    int     x;              /* x position 1 to 80                 */
    int     y;              /* y position 1 to 24                 */

    char    txt[80];        /* Text array to display              */
}txt_t;

extern struct dataSet myData;
static MemoryPool<message_t, 32> mpool;
static MemoryPool<txt_t, 10> tpool;
static Queue<message_t, 32> queue;
static Queue<txt_t, 10> tqueue;
bool displayUp = false;

void displaySendUpdateSensor(int topic, float reading) {
    message_t *message = mpool.try_alloc();
    if(message) {
        message->type =  topic;
        message->value = reading;
        queue.try_put(message);
    }
}
void displayText(char * text, int xPos, int yPos) {
    txt_t *txtMsg = tpool.try_alloc();
    if(txtMsg) {
        strncpy(txtMsg->txt, text, strlen(text));
        txtMsg->x = xPos;
        txtMsg->y = yPos;
        tqueue.try_put(txtMsg);
    }
}

void displayThread(void)
{
    while(!myData.wifiStatus){
        ThisThread::sleep_for(10ms);
    }
    cout << "\033c" ;  // Reset terminal
    ThisThread::sleep_for(500ms);
    cout << "\033)A";  // Select UK Character Set
    ThisThread::sleep_for(100ms);
//    cout << "\033(0";  // Select Graphics set 0
//    ThisThread::sleep_for(10ms);
    cout << "\033[?25l" ;  // Hide Cursor
    ThisThread::sleep_for(100ms);

    while (true) {
        if (myData.updateDisplay) updateDisplay();
        message_t *message;
        auto event = queue.try_get(&message);
        ThisThread::sleep_for(1ms);
        if (event) {
          switch(message->type) {
                case TEMP:
                    std::cout << "\033[2;15H" << std::fixed << std::setw(6)
                        << std::setprecision(1) << (message->value);
                    break;
                case TEMP_SET_VALUE:
                    std::cout << "\033[2;37H" << std::fixed << std::setw(6)
                        << std::setprecision(1) << (message->value);
                    break;
                case HEATER_STATUS:
                    std::cout << "\033[2;63H" << (static_cast<bool>(message->value)?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
                    break;
                case LIGHT:
                    std::cout << "\033[3;15H" << std::fixed << std::setw(6)
                    << std::setprecision(1) << (message->value);
                    break;
                case LIGHT_SET_VALUE:
                    std::cout << "\033[3;37H" << std::fixed << std::setw(6)
                    << std::setprecision(1) << (message->value);
                    break;
                case LIGHT_STATUS:
                    std::cout << "\033[3;63H" << (static_cast<bool>(message->value)?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
                    break;
                default:
                    break;
            }
            mpool.free(message);
        }
        txt_t *txtMsg;
        auto tevent = tqueue.try_get(&txtMsg);
        ThisThread::sleep_for(1ms);
        if (tevent) {
            std::cout << "\033[" << txtMsg->y << ";" << txtMsg->x << "H" << txtMsg->txt;
        
            tpool.free(txtMsg);
 
        }
    }
}
void updateDisplay() {
    std::cout << "\033[2;1H"   // Cursor to 1, 1 (0, 0) HOME
         << "\033[1;37m"
         << "Temperature:         C   Set Temp:         C   Heater Status:      \r\n"
         << "Light Level:         \%   Set Light:        \%   Light Status:       \r\n";
    ThisThread::sleep_for(100ms);
    displaySendUpdateSensor(TEMP_SET_VALUE, myData.tempSet);
    ThisThread::sleep_for(10ms);
    displaySendUpdateSensor(HEATER_STATUS, myData.heaterStatus);
    ThisThread::sleep_for(10ms);
    displaySendUpdateSensor(LIGHT_SET_VALUE, myData.lightSet);
    ThisThread::sleep_for(10ms);
    displaySendUpdateSensor(LIGHT_STATUS, myData.lightStatus);
    ThisThread::sleep_for(10ms);
    myData.updateDisplay = false;
    displayUp = true;
}
