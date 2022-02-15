#include "mbed.h"
#include "display.h"
#include "actuators.h"

extern struct dataSet myData;

void actuatorsThread() {
    DigitalOut lightIndicator(P12_3);
    DigitalOut heatIndicator(P0_5);
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
            displaySendUpdateSensor(LIGHT_STATUS, lightIndicator);
        }
        if (myData.heaterStatus != heatIndicator) {
            myData.heaterStatus = heatIndicator;
            displaySendUpdateSensor(HEATER_STATUS, heatIndicator);
        }


        ThisThread::sleep_for(100ms);
    }

}