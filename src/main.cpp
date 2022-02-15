/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include <mbed.h>
#include <ios>
#include <iostream>
#include <iomanip>
#include "display.h"
#include "sensors.h"
#include "status.h"
#include "actuators.h"

// This is my version of main.cpp Andrew

// Global data packet

struct dataSet myData;

Thread sendingThreadHandle;
Thread displayThreadHandle;
Thread statusRunningThreadHandle;
Thread actuatorThreadHandle;

int main(void)
{

    sendingThreadHandle.start(callback(sendThread));
    displayThreadHandle.start(callback(displayThread));
    statusRunningThreadHandle.start(callback(statusThread));
    actuatorThreadHandle.start(callback(actuatorsThread));
    /* never get here */
}