/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BufferedSerial.h"
#include "NetworkInterface.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "mbed.h"
#include "DFRobot_RGBLCD.h"
#include "nsapi_types.h"
#include "wifi.h"
#include "structs.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     1000ms

DigitalOut led1(LED1);

BufferedSerial pc(USBTX, USBRX, 115200);

DigitalIn button1(PA_1, PullDown);
DigitalIn button2(PA_0, PullDown);
DigitalIn button3(PD_14, PullDown);
DigitalIn button4(PA_3, PullDown);
DigitalIn button5(PA_4, PullDown);

DFRobot_RGBLCD lcd(16, 2, D14, D15);

int buttonMode = 0;
bool inAlarmMode = false;
char test[] = "Hello";
int cursorPos = 15;

void defaultScreen();

void alarmScreen();

void temperatureScreen();

void weatherScreen();

void newsScreen();


int main()
{
    connect_to_BBC();

    lcd.init();

    while(true)
    {
        led1 = !led1;

        if(button1.read() && buttonMode <= 2 && !inAlarmMode)
            buttonMode++;
        else if(button1.read() && !inAlarmMode)
            buttonMode = 0;

        switch(buttonMode)
        {
            case 0:
                if(button2.read())
                    inAlarmMode = !inAlarmMode;

                if(!inAlarmMode)
                    defaultScreen();
                else
                    alarmScreen();
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
    // lcd.setCursor(horizontal, verical)
    // Horizontal: 0-15
    // Vertical: 0-1

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("BBC News:");
    
    lcd.setCursor(cursorPos, 1);
    lcd.printf("%s", test);

    cursorPos--;
    if(cursorPos < 0)
        cursorPos = 15;
}