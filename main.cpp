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

// Blinking rate in milliseconds
#define BLINKING_RATE     350ms
#define WAIT_TIME_MS 1000

DigitalOut led1(LED1);

BufferedSerial pc(USBTX, USBRX, 115200);

DigitalIn button1(PA_1, PullDown);
DigitalIn button2(PA_0, PullDown);
DigitalIn button3(PD_14, PullDown);
DigitalIn button4(PA_3, PullDown);
DigitalIn button5(PA_4, PullDown);

DFRobot_RGBLCD lcd(16, 2, D14, D15);

int unix_time = 0;
int buttonMode = 0;
bool inAlarmMode = false;

void defaultScreen();
void alarmScreen();
void temperatureScreen();
void weatherScreen();
void newsScreen(const char string[], size_t stringSize);

bool temp_state = true;

////FOR TEMP&HUMID///////
InterruptIn button(PA_0, PullDown);
DevI2C I2c(PB_11, PB_10);
HTS221Sensor Sensor(&I2c);
bool temperatureState;
void smart() {
    temp_state = !temp_state;
}    
bool pressed_ones = false;
int option;
float fuktighet;
float temperatur;



int main()
{
    struct NewsStrings *pNews = new NewsStrings;

    // Get the default network instance for all the connections that are required
    NetworkInterface *network = NetworkInterface::get_default_instance();

    // Connect to WorldTime to get UNIX epoch time;
    // WILL BE DONE IN A THREAD LATER
    connect_to_WorldTime(network, unix_time);

    // Shows the epoch time for 5 seconds
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.printf("UNIX epoch time:");
    lcd.setCursor(0, 1);
    lcd.printf("%d", unix_time);
    ThisThread::sleep_for(5000ms);

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
                lcd.setColorWhite();
                weatherScreen();
                break;

            case 3:
                newsScreen(pNews->headlineString, strlen(pNews->headlineString));
                break;

        }
        ThisThread::sleep_for(BLINKING_RATE);
    }
}



void defaultScreen()
{
    lcd.clear();
    lcd.printf("DEFAULT!");
}



void alarmScreen()
{
    lcd.clear();
    lcd.printf("Alarm!");
}



void temperatureScreen()
{
    lcd.clear();

    if (Sensor.init(NULL) != 0) {
        printf("Initialization of device failed\n");
    }

    if (Sensor.enable() !=0) {
        printf("Failed to enable device\n");
    }

        Sensor.get_temperature(&temperatur);
        Sensor.get_humidity(&fuktighet);

        
        button.fall(&smart);
        if (temp_state == true) {
            lcd.clear();
            lcd.setCursor(1,0);
            lcd.printf("Temperatur:");
            lcd.setCursor(0,1);
            lcd.printf(" %.1f", temperatur);
        }
        if (temp_state == false) {
            lcd.clear();
            lcd.setCursor(1,0);
            lcd.printf("Fuktighet:");
            lcd.setCursor(0,1);
            lcd.printf(" %.1f", fuktighet);
        }

        //Temperatur RGB
        if ( temperatur < 20 && temp_state == true) {
            lcd.setColor(BLUE);
        }
        if  (temperatur >= 20 && temperatur <= 24 && temp_state == true) {
            lcd.setRGB(255, 165, 0);
        } 
        if  (temperatur > 24 && temp_state == true) {
            lcd.setColor(RED);
        } 

        //Fuktighet RGB
        if  (fuktighet > 0 && temp_state == false) {
                float fuktighet_meter=2.55*fuktighet;
                lcd.setRGB(255-fuktighet_meter,255-fuktighet_meter,255);
            }

        printf("Fuktighet: %.1f  Temperatur: %.1f\n", fuktighet, temperatur);
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