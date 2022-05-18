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


// Blinking rate in milliseconds
#define BLINKING_RATE     1000ms

DigitalOut led1(LED1);

BufferedSerial pc(USBTX, USBRX, 115200);

InterruptIn button1(PA_1, PullDown);
InterruptIn button2(PA_0, PullDown);
InterruptIn button3(PD_14, PullDown);
InterruptIn button4(PA_3, PullDown);
InterruptIn button5(PA_4, PullDown);

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
    printf("Hallo\n");

    /*---News Feed Get Request---*/
    NetworkInterface *network = NetworkInterface::get_default_instance();

    if(!network)
    {
        printf("Failed to get the default network instance\n");
        while(true);
    }

    nsapi_size_or_error_t result;

    do
    {
        printf("Connecting to network...\n");
        result = network->connect();

        if(result != 0)
        {
            printf("Failed to connect to network: %d\n", result);
        }
    } while(result != 0);

    printf("Connected to network successfully\n");


    SocketAddress address;
    network->get_ip_address(&address);

    TCPSocket *socket = new TCPSocket;

    if(socket == nullptr)
    {
        printf("Failed to allocate socket instance\n");
        while(true);
    }

    socket->open(network);

    const char *host = "http://feeds.bbci.co.uk/news/world/rss.xml#";
    result = network->gethostbyname(host, &address);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of host %s: %d\n", host, result);
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());




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