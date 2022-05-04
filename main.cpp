/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "DFRobot_RGBLCD.h"


// Blinking rate in milliseconds
#define BLINKING_RATE     1000ms

DigitalOut led1(LED1);

InterruptIn button1(PA_1, PullDown);
InterruptIn button2(PA_0, PullDown);

DFRobot_RGBLCD lcd(16, 2, D14, D15);


void defaultScreen();

void alarmScreen();

void temperatureScreen();

void weatherScreen();

void newsScreen();

int main()
{
    lcd.init();

    int buttonMode = 0;

    lcd.printf("Hello!");

    while(true)
    {
        led1 = !led1;

        if(button1.read() && buttonMode <= 3)
            buttonMode++;
        else if(button1.read())
            buttonMode = 0;

        switch(buttonMode)
        {
            case 0:
                defaultScreen();
                break;

            case 1:
                temperatureScreen();
                break;

            case 2:
                weatherScreen();
                break;

            case 3:
                newsScreen();
                break;

        }

        ThisThread::sleep_for(BLINKING_RATE);
    }
}


void defaultScreen()
{
    lcd.clear();
    lcd.printf("Default!");
}

void alarmScreen()
{
    lcd.clear();
    lcd.printf("Alarm!");
}

void temperatureScreen()
{
    lcd.clear();
    lcd.printf("Temperature!");
}

void weatherScreen()
{
    lcd.clear();
    lcd.printf("Weather!");
}

void newsScreen()
{
    lcd.clear();
    lcd.printf("News!");
}