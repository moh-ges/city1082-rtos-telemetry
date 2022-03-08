#include "mbed.h"
#include "display.h"
#include <ios>
#include <iostream>
#include "string.h"
#include <iomanip>
#include <cmath>

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

    char buffer[80];
    while (displayUp == false)
        {
            ThisThread::sleep_for(10ms);
        }
    ThisThread::sleep_for(2s);
    while (true) {
        if (myData.updateDisplay) updateDisplay();

        myData.temperature = readTemp();
       
        myData.lightLevel =  readLight();

        sprintf(buffer, "%2.1f", myData.temperature);
        displayText(buffer, 15, 2);
        sprintf(buffer, "%2.1f", myData.lightLevel);
        displayText(buffer, 15, 3);
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
