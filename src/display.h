#ifndef DISPLAY_H
#define DISPLAY_H

#define TEMP 0
#define TEMP_LOW_THRESH 1
#define TEMP_HIGH_THRESH 2
#define TEMP_SET_VALUE 3
#define LIGHT 10
#define LIGHT_LOW_THRESH 11
#define LIGHT_HIGH_THRESH 12
#define LIGHT_SET_VALUE 13
#define HUMID 20
#define HUMID_LOW_THRESH 21
#define HUMID_HIGH_THRESH 22
#define HUMID_SET_VALUE 23

struct dataSet{
    float temperature;
    float tempSet = 24;
    bool heaterStatus = false;
    float lightLevel;
    float lightSet= 40;
    bool lightStatus = false;
};

void displayThread();
void displaySendUpdateSensor(int, float);


#endif