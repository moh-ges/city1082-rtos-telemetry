/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include <ios>
#include <iostream>
#include <iomanip>
#include "display.h"
#include "vt100.h"

typedef struct {
    int     type;           /* The type of data 0 = Temperature   */
                            /*                  1 = Light Level   */
                            /*                  2 = ToDo          */
    float   value;          /* AD result of measured temperature  */
} message_t;

extern struct dataSet myData;
static MemoryPool<message_t, 32> mpool;
static Queue<message_t, 32> queue;

void displaySendUpdateSensor(int topic, float reading) {
    message_t *message = mpool.try_alloc();
    if(message) {
        message->type =  topic;
        message->value = reading;
        queue.try_put(message);
    }
}

void displayThread(void)
{
    cout << "\033c" ;  // Reset terminal
    ThisThread::sleep_for(100ms);
    cout << "\033)A";  // Select UK Character Set
    ThisThread::sleep_for(10ms);
//    cout << "\033(0";  // Select Graphics set 0
//    ThisThread::sleep_for(10ms);
    cout << "\033[?25l" << endl;  // Clear Display
    ThisThread::sleep_for(100ms);
    std::cout << "\033[H"   // Cursor to 1, 1 (0, 0) HOME
         << "Temperature:               C\r\n"
         << "Light Level:               \%\r\n";
    while (true) {
        message_t *message;
        auto event = queue.try_get(&message);
        ThisThread::sleep_for(1ms);
if (event) {
          switch(message->type) {
                case TEMP:
                    std::cout << "\033[1;21H" << std::fixed << std::setw(6)
                        << std::setprecision(1) << (message->value);
                    break;
                case LIGHT:
                    std::cout << "\033[2;21H" << std::fixed << std::setw(6)
                    << std::setprecision(1) << (message->value);
                    break;
            }
            mpool.free(message);
        }
    }
}
