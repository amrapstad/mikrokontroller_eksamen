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
#include <string.h>

// Blinking rate in milliseconds
#define BLINKING_RATE     350ms

DigitalOut led1(LED1);

BufferedSerial pc(USBTX, USBRX, 115200);

DigitalIn button1(PA_1, PullDown);
DigitalIn button2(PA_0, PullDown);
DigitalIn button3(PD_14, PullDown);
DigitalIn button4(PA_3, PullDown);
DigitalIn button5(PA_4, PullDown);

DFRobot_RGBLCD lcd(16, 2, D14, D15);

char test[] = "Russian oil: EU agrees compromise deal on banning imports";
int cursorPos = 15;
int buttonMode = 0;
bool inAlarmMode = false;

void defaultScreen();

void alarmScreen();

void temperatureScreen();

void weatherScreen();

void newsScreen(char string[], size_t stringSize);

int main()
{
    struct NewsStrings *pNews = new NewsStrings;
    connect_to_BBC(pNews);
    printf("%s\n", pNews->firstString);
    printf("%s\n", pNews->secondString);
    printf("%s\n", pNews->thirdString);

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
                newsScreen(test, strlen(test));
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

void newsScreen(char newsString[], size_t stringSize)
{
    // lcd.setCursor(horizontal, vertical)
    // Horizontal: 0-15
    // Vertical: 0-1
    int totalColumns = 16;
    static int newsStringBufferStart = 0;
    static int newsStringBufferEnd = 0;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.printf("BBC News:");

    lcd.setCursor(cursorPos, 1);

    // Prints string when the first letter doesn't touch the left side
    if(cursorPos > 0)
    {
        for(int i = 0; i < newsStringBufferEnd + 1; i++)
        {
            lcd.printf("%c", newsString[i]);
            cursorPos++;
        }
        newsStringBufferEnd++;
        cursorPos -= newsStringBufferEnd + 1;
    }
    else if(newsStringBufferStart < stringSize)
    {
        // Prints string when it fill the whole display 
        if(newsStringBufferEnd < stringSize)
        {
            for(int i = 0; i < totalColumns; i++)
            {
                lcd.printf("%c", newsString[newsStringBufferStart + i]);
                cursorPos++;
            }
        }
        // Prints string when the last letter doesn't touch the right side
        else
        {
            for(int i = 0; i < stringSize - newsStringBufferStart; i++)
            {
                lcd.printf("%c", newsString[ newsStringBufferStart + i]);
            }
        }
        newsStringBufferStart++;
        newsStringBufferEnd++;
        cursorPos = 0;
    }
    else
        return;
}