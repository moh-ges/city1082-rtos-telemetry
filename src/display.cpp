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

#define CLEAR "CLS"
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
static MemoryPool<txt_t, 32> tpool;
static Queue<txt_t, 32> tqueue;
bool displayUp = false;

void displayText(char * text, int xPos, int yPos) {
    txt_t *txtMsg = tpool.try_alloc();
    if(txtMsg) {
        strcpy(txtMsg->txt, text);
        txtMsg->x = xPos;
        txtMsg->y = yPos;
        tqueue.try_put(txtMsg);
    }
}

void displayThread(void)
{
    cout << "\033c" ;  // Reset terminal
    ThisThread::sleep_for(200ms);
    cout << "\033)A";  // Select UK Character Set
    ThisThread::sleep_for(10ms);
    cout << "\033[?25l" ;  // Hide Cursor
    while (true) {
        txt_t *txtMsg;
        auto tevent = tqueue.try_get(&txtMsg);
        ThisThread::sleep_for(1ms);
        if (tevent) {
            if ((txtMsg->txt[0] == 'C')&&
                (txtMsg->txt[1] == 'L')&&
                (txtMsg->txt[2] == 'S')) 
            {
                printf("\033[2J");
            } 
            else {
                std::cout << "\033[" << txtMsg->y << ";" << txtMsg->x << "H" << txtMsg->txt;
            }
            
        
            tpool.free(txtMsg);
 
        }
    }
}
void initDisplay() {
    displayText( CLEAR, 1, 1); // clear Screen
    updateDisplay(); // initialise readings/status display
    displayUp = true;
}
void updateDisplay() {
    char buffer[80];
    displayText( "\033[0;33mTemperature:\033[K", 1, 2);
    displayText( "C", 21, 2);
    displayText( "Set Temp", 26, 2);
    displayText( "C", 43, 2);
    displayText( "Heater Status:", 48, 2);
    displayText( "Light Level:\033[K", 1, 3);
    displayText( "%", 21, 3);
    displayText( "Set Light", 26, 3);
    displayText( "%", 43, 3);
    displayText( "Light Status:\033[?25l", 48, 3);
    displayText( "Sub Count:", 44, 12);
    displayText( "Pub Count:", 44, 13);
    sprintf(buffer, "\033[1;33m%2.1f", myData.tempSet);
    displayText(buffer, 37, 2);
    sprintf(buffer, "%s", myData.heaterStatus?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
    displayText(buffer, 63, 2);
    sprintf(buffer, "\033[1;33m%2.1f", myData.lightSet);
    displayText(buffer, 37, 3);
    sprintf(buffer, "%s", myData.lightStatus?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
    displayText(buffer, 63, 3);

    myData.updateDisplay = false;
}
