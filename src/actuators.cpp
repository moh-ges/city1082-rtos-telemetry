#include "mbed.h"
#include "display.h"
#include "actuators.h"

extern struct dataSet myData;
extern bool displayUp;

void actuatorsThread() {
    char buffer[80];
    DigitalOut lightIndicator(P12_3);
    DigitalOut heatIndicator(P0_5);
    while (displayUp == false) {
        ThisThread::sleep_for(10ms);
    }
    while(true){
        if (myData.lightLevel < myData.lightSet - myData.lightThresh) {
            lightIndicator = true;
        }
        else if (myData.lightLevel > myData.lightSet + myData.lightThresh) {
            lightIndicator = false;
        }
        if (myData.temperature < myData.tempSet - myData.tempThresh) {
            heatIndicator = true;
        }
        else if (myData.temperature > myData.tempSet + myData.tempThresh) {
            heatIndicator = false;
        }
        if (myData.lightStatus != lightIndicator) {
            myData.lightStatus = lightIndicator;
            sprintf(buffer, "%s", lightIndicator?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
            displayText(buffer, 63, 3);
        }
        if (myData.heaterStatus != heatIndicator) {
            myData.heaterStatus = heatIndicator;
            sprintf(buffer, "%s", heatIndicator?
                    "\033[1;31mON  \033[1;37m":"\033[1;32mOFF\033[1;37m");
            displayText(buffer, 63, 2);
        }
        ThisThread::sleep_for(100ms);
    }
}