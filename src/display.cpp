/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include <ios>
#include <iostream>
#include <iomanip>
#include "display.h"

typedef struct {
    float    temperature;   /* AD result of measured temperature  */
    // adding to test github
    float    lightLevel;    /* AD result of measured lightLevel   */
    int counter;            /* A counter value                    */
} message_t;

static MemoryPool<message_t, 32> mpool;
static Queue<message_t, 32> queue;

void displaySendUpdateSensor(float temperature, float lightLevel, int cycles) {
    message_t *message = mpool.try_alloc();
    if(message) {
        message->temperature =  temperature;
        message->lightLevel = lightLevel;
        message->counter = cycles;
        queue.try_put(message);
    }
}

void displayThread(void)
{
    cout << "\033c" << "\033[?25l" << endl;
    std::cout << "\033[H" 
         << "Temperature:               C\r\n"
         << "Light Level:               \%\r\n"
         << "Number of Cycles\r\n";
    while (true) {
        message_t *message;
      auto event = queue.try_get(&message);
      if (event) {
        std::cout << "\033[1;21H" << std::fixed << std::setw(6)
                  << std::setprecision(1) << (message->temperature);
        std::cout << "\033[2;21H" << std::fixed << std::setw(6)
                  << std::setprecision(1) << (message->lightLevel);
        std::cout << "\033[3;19H"
                  << std::setw(6)
                  << (message->counter) << std::endl;
        mpool.free(message);
        }
    }
}
