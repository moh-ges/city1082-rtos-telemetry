#include "mbed.h"
#include "display.h"
#include <cmath>

#define THERM_GND P10_0
#define THERM_VCC P10_3
#define THERM_OUT P10_1

/* Send Thread */

void sendThread(void)
{
    // this is for reading the thermistor
    DigitalOut ground(THERM_GND);
    DigitalOut vcc(THERM_VCC);
    AnalogIn tempVoltage(THERM_OUT);
    // making a change

    float    temp;  // AD result of measured voltage 
    float    lightLev;   // AD result of measured current
    int      cycles;       // A counter value               
    uint32_t i = 0;
    while (true) {
        i++; // fake data update
        temp = fmod((i * 0.1f/*the temp */) * \
                    3.3f, 40);
        lightLev = fmod((i * 0.1f) * 5.5f, 100);
        cycles = i;
        displaySendUpdateSensor(temp, lightLev, cycles);
        ThisThread::sleep_for(1s);
        
    }
}
