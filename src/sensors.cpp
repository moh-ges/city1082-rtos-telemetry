#include "mbed.h"
#include "display.h"
#include <cmath>

#define THERM_GND P10_0
#define THERM_VCC P10_3
#define THERM_OUT P10_1

/* Reference resistor in series with the thermistor is of 10kohm */
#define R_REFERENCE                         (float)(10000)

/* Beta constant of NCP18XH103F03RB thermistor is 3380 Kelvin. See the thermistor
   data sheet for more details. */
#define B_CONSTANT                          (float)(3380)

/* Resistance of the thermistor is 10K at 25 degrees C (from the data sheet)
   Therefore R0 = 10000 Ohm, and T0 = 298.15 Kelvin, which gives
   R_INFINITY = R0 e^(-B_CONSTANT / T0) = 0.1192855 */
#define R_INFINITY                          (float)(0.1192855)

/* Zero Kelvin in degree C */
#define ABSOLUTE_ZERO                       (float)(-273.15)

AnalogIn tempVoltage(THERM_OUT);

/* Send Thread */
float readTemp();

void sendThread(void)
{
    // this is for reading the thermistor
    DigitalOut ground(THERM_GND);
    DigitalOut vcc(THERM_VCC);
    // initialise the thermistor power
    vcc= false;
    ground = true;

    float    temp;  // AD result of measured voltage 
    float    lightLev;   // AD result of measured current
    int      cycles;       // A counter value               
    uint32_t i = 0;
    while (true) {
        i++; // fake data update
/*        temp = tempVoltage.read() * 3.3f; */
        float temperature;
/*        float rThermistor; */

        /* Calculate the thermistor resistance */
//        rThermistor = tempVoltage.read() * R_REFERENCE;

        /* Calculate the temperature in deg C */
//        temperature = (B_CONSTANT/(logf(rThermistor/R_INFINITY))) + ABSOLUTE_ZERO;
        temperature = readTemp();
        lightLev = fmod((i * 0.1f) * 5.5f, 100);
        cycles = i;
        displaySendUpdateSensor(temperature, lightLev, cycles);
        ThisThread::sleep_for(1s);
        
    }
}

float readTemp()
{
    float refVoltage = tempVoltage.read() * 2.4; // Range of ADC 0->2*Vref
    float refCurrent = refVoltage  / 10000.0; // 10k Reference Resistor
    float thermVoltage = 3.3 - refVoltage;    // Assume supply voltage is 3.3v
    float thermResistance = thermVoltage / refCurrent; 
    float logrT = (float32_t)log((float64_t)thermResistance);
    /* Calculate temperature from the resistance of thermistor using Steinhart-Hart Equation */
    float stEqn = (float32_t)((0.0009032679) + ((0.000248772) * logrT) + 
                             ((2.041094E-07) * pow((float64)logrT, (float32)3)));
    float temperatureC = (float32_t)(((1.0 / stEqn) - 273.15)  + 0.5);
    return temperatureC;
}
