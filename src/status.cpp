#include "mbed.h"
#include <ios>
#include <iostream>
#include <iomanip>
#include "status.h"
#include "display.h"

extern struct dataSet myData;

void statusThread() {
    DigitalOut statusLed(P13_7);
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [128];
    while (true) {
        
        statusLed = !statusLed;
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        strftime (buffer,sizeof(buffer),"%c ",timeinfo);
        displayText(buffer, 1, 8);
        ThisThread::sleep_for(500ms);

    }
}
