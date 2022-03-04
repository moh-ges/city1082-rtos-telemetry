#include "mbed.h"
#include "display.h"
#include <ios>
#include <iostream>
#include "string.h"
#include <iomanip>
#include <cmath>

void updateDisplay();
//#define THERM_GND P10_3
//#define THERM_VCC P10_0
#define THERM_OUT P10_1



/* Reference resistor in series with the thermistor is of 10kohm */
#define R_REFERENCE                         (float)(10000)

/* A constant of NCP18XH103F03RB thermistor is */
#define A_COEFF                          (float)(0.0009032679f)
/* B constant of NCP18XH103F03RB thermistor is */
#define B_COEFF                          (float)(0.000248772f)
/* C constant of NCP18XH103F03RB thermistor is */
#define C_COEFF                          (float)(0.0000002041094f)

/* Zero Kelvin in degree C */
#define ABSOLUTE_ZERO                       (float)(-273.15)

#define LDR_PORT    P10_4

AnalogIn tempVoltage(THERM_OUT);
AnalogIn lightLevel(LDR_PORT);

extern struct dataSet myData;
extern bool displayUp;
/* Send Thread */
float readTemp();
float readLight();

void sendThread(void)
{
    // this is for reading the thermistor
    // DigitalOut ground(THERM_GND); // Now permanent connection to power
    // DigitalOut vcc(THERM_VCC);    // so redundant.
    // initialise the thermistor power
    // vcc= false;
    // ground = true;

    //float    temp;  // AD result of measured voltage 
    //float    lightLev;   // AD result of measured current
    //int      cycles;       // A counter value               
    // uint32_t i = 0;
    while (displayUp == false)
        {
            ThisThread::sleep_for(10ms);
        }
    updateDisplay();

    while (true) {
    //    i++; // fake data update
        //float temperature;
        if (myData.updateDisplay) updateDisplay();

        myData.temperature = readTemp();
       
        myData.lightLevel =  readLight();

    //    cycles = i;
        displaySendUpdateSensor(TEMP, myData.temperature);
        displaySendUpdateSensor(LIGHT, myData.lightLevel);
        ThisThread::sleep_for(100ms);
        
    }
}

float readTemp()
{
    float refVoltage = tempVoltage.read() * 2.4; // Range of ADC 0->2*Vref
    float refCurrent = refVoltage  / R_REFERENCE; // 10k Reference Resistor
    float thermVoltage = 3.3 - refVoltage;    // Assume supply voltage is 3.3v
    float thermResistance = thermVoltage / refCurrent; 
    float logrT = (float32_t)log((float64_t)thermResistance);
    /* Calculate temperature from the resistance of thermistor using Steinhart-Hart Equation */
    float stEqn = (float32_t)((A_COEFF) + ((B_COEFF) * logrT) + 
                             ((C_COEFF) * pow((float64)logrT, (float32)3)));
    float temperatureC = (float32_t)(((1.0 / stEqn) + ABSOLUTE_ZERO)  + 0.05);
    return temperatureC;
}

float readLight()
{
    return (lightLevel.read())*100.0f;
}
void updateDisplay() {
    cout << "\033[1;37m";
    ThisThread::sleep_for(100ms);
    displayText( "Temperature:", 1, 2);
    displayText( "C", 22, 2);
    displayText( "Set Temp", 26, 2);
    displayText( "C", 44, 2);
    displayText( "Heater Status:", 48, 2);
    displayText( "Light Level:", 1, 3);
    displayText( "%", 22, 3);
    displayText( "Set Light", 26, 3);
    displayText( "%", 44, 3);
    displayText( "Light Status:", 48, 3);
//    displayText( "Temperature:         C   Set Temp:         C   Heater Status:      ", 1, 2);
//    ThisThread::sleep_for(10ms);
//    displayText( "Light Level:         %   Set Light:        %   Light Status:       ", 1, 3);
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
