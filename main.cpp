/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"


// Blinking rate in milliseconds
#define BLINKING_RATE     500ms


int main()
{
    int ButtonOption;
    bool clock;
    bool alarm;
    bool temp_hum;
    bool weather;

        while(true) {

        if (ButtonOption == 5) {
            ButtonOption = 1;
        }


        if (ButtonOption == 1) {
            clock = true;
            alarm = false;
            temp_hum = false;
            weather = false;
        }
        if (ButtonOption == 2) {
            alarm = true;
            temp_hum = false;
            weather = false;
            clock = false;
        }
        if (ButtonOption == 3) {
            temp_hum = true;
            alarm = false;
            temp_hum = false;
            weather = false;
        }
        if (ButtonOption == 4) {
            weather = true;
            temp_hum = false;
            alarm = false;
            temp_hum = false;
        }



        while(clock == true) {
            clock_function();
        }

        while(clock == true) {
            alarm_function();
        }

        while(clock == true) {
            temp_hum_function();
        }

        while(clock == true) {
            weaher_function();
        }



    }


}
