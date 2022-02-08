#include "mbed.h"
#include <ios>
#include <iostream>
#include <iomanip>
#include "status.h"
#include "display.h"

extern struct dataSet myData;

void statusThread() {
    DigitalOut statusLed(P13_7);
    while (true) {
        
        statusLed = !statusLed;

        ThisThread::sleep_for(500ms);

    }
}
