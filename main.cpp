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
#define REFRESH_RATE       100ms
#define BUF_LENGTH          256


////DEVICES////
DigitalOut led1(LED1);

BufferedSerial pc(USBTX, USBRX, 115200);

DigitalIn button1(PA_1, PullDown);
DigitalIn button2(PA_0, PullDown);
DigitalIn button3(PD_14, PullDown);
DigitalIn button4(PA_3, PullDown);
DigitalIn button5(PA_4, PullDown);

PwmOut buzzer(D9);

DFRobot_RGBLCD lcd(16, 2, D14, D15);

DevI2C i2c_device(PB_11, PB_10);
HTS221Sensor sensor(&i2c_device);

////GLOBAL BARIABLES////
int unix_time = 0;
time_t rtc_timer;
int buttonMode = 0;
bool in_alarm_screen = false;
bool inTemperatureState = true;
float humidity;
float temperature;
int current_hour = 0;
int current_minute = 0;

struct Alarm
{
    int hour;
    int minute;

    bool turned_on;
    bool enabled;
    bool sounding_alarm;
};


////STANDARD FUNCTIONS////
void defaultScreen(char *time_buffer, struct tm *time_struct, struct Alarm &alarm_struct);
void alarmScreen(struct Alarm &alarm_struct);
void temperatureScreen();
void weatherScreen();
void newsScreen(const char string[], size_t stringSize);
void sound_the_alarm();


int main()
{
    ////STACK/HEAP VARIABLES////
    struct Alarm alarm_struct;
    struct NewsStrings *pNews = new NewsStrings;
    NetworkInterface *network = NetworkInterface::get_default_instance();

    // RTC time that we will use to display current time and etc.
    char time_buffer[BUF_LENGTH] = { 0 };
    struct tm *time_struct = nullptr;

    ////INITIALIZE////
    alarm_struct.hour = alarm_struct.minute = 0;
    alarm_struct.turned_on = alarm_struct.sounding_alarm = false;
    alarm_struct.enabled = true;
    buzzer.write(0.f);

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
        current_hour = time_struct->tm_hour;
        current_minute = time_struct->tm_min;

        // Sound the alarm when the current time mathces the alarm time
        if(alarm_struct.turned_on && alarm_struct.enabled && current_hour == alarm_struct.hour && current_minute == alarm_struct.minute)
        {
            sound_the_alarm();
            alarm_struct.sounding_alarm = true;
        }
        else
            buzzer.write(0.f);

        led1 = !led1;

        if(button1.read() && buttonMode <= 2 && !in_alarm_screen)
            buttonMode++;
        else if(button1.read() && !in_alarm_screen)
            buttonMode = 0;

        switch(buttonMode)
        {
            case 0:
                if(in_alarm_screen && button2.read())
                    alarm_struct.turned_on = true;
                if(button2.read())
                    in_alarm_screen = !in_alarm_screen;

                if(!in_alarm_screen)
                    defaultScreen(time_buffer, time_struct, alarm_struct);
                else
                    alarmScreen(alarm_struct);
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



void defaultScreen(char *time_buffer, struct tm *time_struct, struct Alarm &alarm_struct)
{
    strftime(time_buffer, BUF_LENGTH, "%a %d %b %H:%M", time_struct);

    // Will turn off the alarm
    if(alarm_struct.sounding_alarm && button3.read())
    {
        alarm_struct.sounding_alarm = false;
        alarm_struct.enabled = false;
    }

    // Will snooze by adding 5 minutes
    if(alarm_struct.sounding_alarm && button4.read())
    {
        alarm_struct.sounding_alarm = false;
        alarm_struct.minute += 5;
        if(alarm_struct.minute >= 60)
        {
            alarm_struct.minute -= 60;

            alarm_struct.hour++;
            if(alarm_struct.hour >= 24)
                alarm_struct.hour = 0;               
        }
    }

    // Will delete the current alarm
    if(button5.read())
    {
        alarm_struct.turned_on = false;
        alarm_struct.hour = 0;
        alarm_struct.minute = 0;
    }

    // Different outcomes depending on the alarm state
    // First row is always
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("%s", time_buffer);
    lcd.setCursor(0, 1);
    // Active alarm set will show time
    if(alarm_struct.turned_on)
    {
        // Will show only time
        if(alarm_struct.enabled)
        {
            if(alarm_struct.hour < 10)
            {
                if(alarm_struct.minute < 10)
                    lcd.printf("Alarm 0%d:0%d", alarm_struct.hour, alarm_struct.minute); 
                else
                    lcd.printf("Alarm 0%d:%d", alarm_struct.hour, alarm_struct.minute);
            }
            else
            {
                if(alarm_struct.minute < 10)
                    lcd.printf("Alarm %d:0%d", alarm_struct.hour, alarm_struct.minute);
                else
                    lcd.printf("Alarm %d:%d", alarm_struct.hour, alarm_struct.minute);
            }
        }
        // Will have the "OFF" text between "Alarm" and time
        else
        {
            if(alarm_struct.hour < 10)
            {
                if(alarm_struct.minute < 10)
                    lcd.printf("Alarm Off 0%d:0%d", alarm_struct.hour, alarm_struct.minute); 
                else
                    lcd.printf("Alarm Off 0%d:%d", alarm_struct.hour, alarm_struct.minute);
            }
            else
            {
                if(alarm_struct.minute < 10)
                    lcd.printf("Alarm Off %d:0%d", alarm_struct.hour, alarm_struct.minute);
                else
                    lcd.printf("Alarm Off %d:%d", alarm_struct.hour, alarm_struct.minute);
            }
        }
            
    }
    // Will only show "Alarm" on the screen since there isn't any alarm enabled
    else
        lcd.printf("Alarm");
}



void alarmScreen(struct Alarm &alarm_struct)
{
    // Will add one hour to the alarm
    if(button3.read())
    {
        alarm_struct.hour++;
        if(alarm_struct.hour >= 24)
            alarm_struct.hour = 0;
    }

    // Will add one minute to the alarm
    if(button4.read())
    {
        if(alarm_struct.minute >= 59)
        {
            alarm_struct.minute = 0;
            alarm_struct.hour++;
            if(alarm_struct.hour >= 24)
                alarm_struct.hour = 0;
        }
        else
            alarm_struct.minute++;
    }

    // Will enable/disable the alarm
    // Disabled means that the alarm will not sound if the clock reaches the alarm time, but the alarm is still active
    // Indicated by "On" or "Off" on the display
    if(button5.read())
        alarm_struct.enabled = !alarm_struct.enabled;

    lcd.clear();
    lcd.setCursor(0, 0);
    if(alarm_struct.hour < 10)
    {
        if(alarm_struct.minute < 10)
            lcd.printf("Alarm 0%d:0%d", alarm_struct.hour, alarm_struct.minute); 
        else
            lcd.printf("Alarm 0%d:%d", alarm_struct.hour, alarm_struct.minute);
    }
    else
    {
        if(alarm_struct.minute < 10)
            lcd.printf("Alarm %d:0%d", alarm_struct.hour, alarm_struct.minute);
        else
            lcd.printf("Alarm %d:%d", alarm_struct.hour, alarm_struct.minute);
    }
    lcd.setCursor(0, 1);
    if(alarm_struct.enabled)
        lcd.printf("On");
    else
        lcd.printf("Off");
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



void sound_the_alarm()
{
    buzzer.write(0.5f);
    buzzer.period(0.01f);
}