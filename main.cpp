#include "mbed.h"
#include "DFRobot_RGBLCD.h"
#include "nsapi_types.h"
#include "wifi.h"
#include <string.h>
#include "BufferedSerial.h"
#include "NetworkInterface.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "HTS221Sensor.h"

// Define numbers
#define REFRESH_RATE       1000ms
#define BUF_LENGTH          256


////DEVICES////
DigitalOut led1(LED1);

BufferedSerial pc(USBTX, USBRX, 115200);

DigitalIn button1(PA_1, PullDown);
DigitalIn button2(PA_0, PullDown);
DigitalIn button3(PD_14, PullDown);
DigitalIn button4(PA_3, PullDown);
DigitalIn button5(PA_4, PullDown);

DFRobot_RGBLCD lcd(16, 2, D14, D15);

DevI2C i2c_device(PB_11, PB_10);
HTS221Sensor sensor(&i2c_device);


////GLOBAL BARIABLES////
int unix_time = 0;
time_t rtc_timer;
int buttonMode = 0;
bool inAlarmMode = false;
bool inTemperatureState = true;
float humidity;
float temperature;


////STANDARD FUNCTIONS////
void defaultScreen(char *time_buffer, struct tm *time_struct);
void alarmScreen();
void temperatureScreen();
void weatherScreen();
void newsScreen(const char string[], size_t stringSize);


int main()
{
    ////STACK/HEAP VARIABLES////
    struct NewsStrings *pNews = new NewsStrings;
    NetworkInterface *network = NetworkInterface::get_default_instance();

    // RTC time that we will use to display current time and etc.
    char time_buffer[BUF_LENGTH] = { 0 };
    struct tm *time_struct = nullptr;



    if(!network)
    {
        printf("Failed to get the default network instance\n");
        while(true);
    }

    // Connect to BBCs RSS feed to get news headlines
    // WILL BE DONE IN A THREAD LATER
    connect_to_BBC(network, pNews);

    // Connect to WorldTime to get UNIX epoch time;
    // WILL BE DONE IN A THREAD LATER
    network = NetworkInterface::get_default_instance();
    connect_to_WorldTime(network, unix_time);

    // Since the epoch time is UTC/GMT, we need to adjust so it mathces our timezone
    // We do this by adding 2 hours or 7200 seconds (60 * 60 * 2 = 7200) to the epcoh time
    set_time(unix_time + 7200);

    // Will show the epoch time for 5 seconds and initialize the display
    // The last print will print the actual current epoch time by subtracting the offset we added earlier
    lcd.init();
    int time_end = unix_time + 7200 + 5;
    while(rtc_timer < time_end)
    {
        rtc_timer = time(NULL);
        lcd.setCursor(0, 0);
        lcd.printf("UNIX epoch time:");
        lcd.setCursor(0, 1);
        lcd.printf("%d", rtc_timer - 7200);
        ThisThread::sleep_for(REFRESH_RATE);
    }
    lcd.clear();

    while(true)
    {
        rtc_timer = time(NULL);
        time_struct = localtime(&rtc_timer);

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
                    defaultScreen(time_buffer, time_struct);
                else
                    alarmScreen();
                break;

            case 1:
                if(button2.read())
                    inTemperatureState = !inTemperatureState;
                
                temperatureScreen();
                break;

            case 2:
                lcd.setColorWhite();
                weatherScreen();
                break;

            case 3:
                newsScreen(pNews->headlineString, strlen(pNews->headlineString));
                break;

        }
        ThisThread::sleep_for(REFRESH_RATE);
    }
}



void defaultScreen(char *time_buffer, struct tm *time_struct)
{
    strftime(time_buffer, BUF_LENGTH, "%a %d %b %H:%M", time_struct);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("%s", time_buffer);
    lcd.setCursor(0, 1);
    lcd.printf("Alarm");
}



void alarmScreen()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Alarm ");
    lcd.setCursor(0, 1);
    lcd.printf("OFF");
}



void temperatureScreen()
{
    lcd.clear();

    if (sensor.init(NULL) != 0) {
        printf("Initialization of device failed\n");
    }

    if (sensor.enable() !=0) {
        printf("Failed to enable device\n");
    }

        sensor.get_temperature(&temperature);
        sensor.get_humidity(&humidity);
       
        if (inTemperatureState)
        {
            lcd.clear();
            lcd.setCursor(1,0);
            lcd.printf("Temperature:");
            lcd.setCursor(0,1);
            lcd.printf(" %.1f C", temperature);
        }
        else if(!inTemperatureState)
        {
            lcd.clear();
            lcd.setCursor(1,0);
            lcd.printf("Fuktighet:");
            lcd.setCursor(0,1);
            lcd.printf(" %.1f%%", humidity);
        }

        //Temperature RGB
        if (temperature < 20 && inTemperatureState) {
            lcd.setColor(BLUE);
        }
        if  (temperature >= 20 && temperature <= 24 && inTemperatureState) {
            lcd.setRGB(255, 165, 0);
        } 
        if  (temperature > 24 && inTemperatureState) {
            lcd.setColor(RED);
        } 

        //Humidity RGB
        if  (humidity > 0 && !inTemperatureState)
        {
            float humidity_meter=2.55*humidity;
            lcd.setRGB(255-humidity_meter, 255-humidity_meter, 255);
        }
}



void weatherScreen()
{
    lcd.clear();
    lcd.printf("Weather!");
}



void newsScreen(const char inputString[], size_t stringSize)
{
    // lcd.setCursor(horizontal, vertical)
    // Horizontal: 0-15
    // Vertical: 0-1
    int totalColumns = 16;
    static int cursorPos = 15;
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
            lcd.printf("%c", inputString[i]);
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
                lcd.printf("%c", inputString[newsStringBufferStart + i]);
                cursorPos++;
            }
        }
        // Prints string when the last letter doesn't touch the right side
        else
        {
            for(int i = 0; i < stringSize - newsStringBufferStart; i++)
            {
                lcd.printf("%c", inputString[ newsStringBufferStart + i]);
            }
        }
        newsStringBufferStart++;
        newsStringBufferEnd++;
        cursorPos = 0;
    }
    else
    {
        // Resets scrolling
        cursorPos = 15;
        newsStringBufferStart = 0;
        newsStringBufferEnd = 0;
        return;
    }
}