#include "mbed.h"
#include "DFRobot_RGBLCD.h"
#include "HTS221Sensor.h"
#include "ip_geo.h"


#define JSON_NOEXCEPTION
#include "json.hpp"

using json = nlohmann::json;


const char *get_nsapi_error_string(nsapi_error_t err);

nsapi_size_or_error_t send_request(Socket *socket, const char *request);

nsapi_size_or_error_t read_response(Socket *socket, char *buffer, int buffer_length);


DigitalOut led1(LED1);

DigitalIn button1(PA_1, PullDown);
DigitalIn button2(PA_0, PullDown);
DigitalIn button3(PD_14, PullDown);
DigitalIn button4(PA_3, PullDown);
DigitalIn button5(PA_4, PullDown);

DFRobot_RGBLCD lcd(16, 2, D14, D15);

int buttonMode = 0;
bool inAlarmMode = false;

#define BLINKING_RATE 1000ms
#define SLEEP_TIME 3000ms

void defaultScreen();

void alarmScreen();

void temperatureScreen();

void weatherScreen();

void newsScreen();

void startNetwork();

int main()
{
    lcd.init();

    DigitalOut led(LED1);








//////////////////////////////////START NETWORK/////////////////////////////////////////////////////

NetworkInterface *network = NetworkInterface::get_default_instance();

  if (!network) {
    printf("Failed to get default network interface\n");
    while (1);
  }

  nsapi_size_or_error_t result;

  do {
    printf("Trying to connect to the network...\n");
    result = network->connect();

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to connect to network: %d\n", result);
    }
  } while (result != NSAPI_ERROR_OK);

  SocketAddress address;
  result = network->get_ip_address(&address);

  if (result != NSAPI_ERROR_OK) {
    printf("Failed to get local IP address: %s\n",
           get_nsapi_error_string(result));
    while (1);
  }

  printf("Connected to WLAN and got IP address %s\n", address.get_ip_address());

  while (true) {
    led = !led;
    ThisThread::sleep_for(BLINKING_RATE);

    TLSSocket socket;
    socket.set_timeout(500);

    result = socket.open(network);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to open TLSSocket: %s\n", get_nsapi_error_string(result));
      continue;
    }

    const char host[] = "ipgeolocation.io";
    result = network->gethostbyname(host, &address);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to get IP address of host %s: %s\n", host,
             get_nsapi_error_string(result));
      continue;
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());

    address.set_port(443);

    result = socket.set_root_ca_cert(ip_geo_cert);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to set root certificate of the web site: %s\n",
             get_nsapi_error_string(result));
      continue;
    }

    // Connect to server at the given address
    result = socket.connect(address);

    // Check result
    if (result != NSAPI_ERROR_OK) {
      printf("Failed to connect to server at %s: %s\n", host,
             get_nsapi_error_string(result));
      continue;
    }

    printf("Successfully connected to server %s\n", host);

    // Create HTTP request
    const char request[] = "GET Europe/Oslo?format=json HTTP/1.1\r\n"
                           "Host: ipgeolocation.io\r\n"
                           "Connection: close\r\n"
                           "\r\n";

    result = send_request(&socket, request);

        if (result < 0) {
      printf("Failed to send request: %d\n", result);
      continue;
    }

    static char buffer[2000];

    
    char *json_begin = strchr(buffer, '{');
    char *json_end = strrchr(buffer, '}');


        // Check if we actually got JSON in the response
    if (json_begin == nullptr || json_end == nullptr) {
      printf("Failed to find JSON in response\n");
      continue;
    }

    // End the string after the end of the JSON data in case the response
    // contains trailing data
    json_end[1] = 0;

    printf("JSON response:\n");
    printf("%s\n", json_begin);

    // Parse response as JSON, starting from the first {

    result = read_response(&socket, buffer, sizeof(buffer));

        if (result < 0) {
      printf("Failed to read response: %d\n", result);
      continue;
    }

    json document = json::parse(json_begin);

    if (document.is_discarded()) {
      printf("The input is invalid JSON\n");
      continue;
    }

    // Get IP address from JSON object
    std::string name;
    document["name"].get_to(name);

    printf("name: %s\n", name.c_str());
  }
    ////////////////////////////////////////////////END NETWORK/////////////////////////////////////////////////////////////////










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
    return 0;
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


















const char *get_nsapi_error_string(nsapi_error_t err) {
  switch (err) {
  case NSAPI_ERROR_OK:
    return "NSAPI_ERROR_OK";
  case NSAPI_ERROR_WOULD_BLOCK:
    return "NSAPI_ERROR_WOULD_BLOCK";
  case NSAPI_ERROR_UNSUPPORTED:
    return "NSAPI_ERROR_UNSUPPORTED";
  case NSAPI_ERROR_PARAMETER:
    return "NSAPI_ERROR_PARAMETER";
  case NSAPI_ERROR_NO_CONNECTION:
    return "NSAPI_ERROR_NO_CONNECTION";
  case NSAPI_ERROR_NO_SOCKET:
    return "NSAPI_ERROR_NO_SOCKET";
  case NSAPI_ERROR_NO_ADDRESS:
    return "NSAPI_ERROR_NO_ADDRESS";
  case NSAPI_ERROR_NO_MEMORY:
    return "NSAPI_ERROR_NO_MEMORY";
  case NSAPI_ERROR_NO_SSID:
    return "NSAPI_ERROR_NO_SSID";
  case NSAPI_ERROR_DNS_FAILURE:
    return "NSAPI_ERROR_DNS_FAILURE";
  case NSAPI_ERROR_DHCP_FAILURE:
    return "NSAPI_ERROR_DHCP_FAILURE";
  case NSAPI_ERROR_AUTH_FAILURE:
    return "NSAPI_ERROR_AUTH_FAILURE";
  case NSAPI_ERROR_DEVICE_ERROR:
    return "NSAPI_ERROR_DEVICE_ERROR";
  case NSAPI_ERROR_IN_PROGRESS:
    return "NSAPI_ERROR_IN_PROGRESS";
  case NSAPI_ERROR_ALREADY:
    return "NSAPI_ERROR_ALREADY";
  case NSAPI_ERROR_IS_CONNECTED:
    return "NSAPI_ERROR_IS_CONNECTED";
  case NSAPI_ERROR_CONNECTION_LOST:
    return "NSAPI_ERROR_CONNECTION_LOST";
  case NSAPI_ERROR_CONNECTION_TIMEOUT:
    return "NSAPI_ERROR_CONNECTION_TIMEOUT";
  case NSAPI_ERROR_ADDRESS_IN_USE:
    return "NSAPI_ERROR_ADDRESS_IN_USE";
  case NSAPI_ERROR_TIMEOUT:
    return "NSAPI_ERROR_TIMEOUT";
  case NSAPI_ERROR_BUSY:
    return "NSAPI_ERROR_BUSY";
  default:
    return "NSAPI_ERROR_UNKNOWN";
  }
}

nsapi_size_or_error_t send_request(Socket *socket, const char *request) {
  if (socket == nullptr || request == nullptr) {
    printf("Invalid function parameters\n");
    return NSAPI_ERROR_PARAMETER;
  }

  // The request might not be fully sent in one go,
  // so keep track of how much we have sent
  nsapi_size_t bytes_to_send = strlen(request);
  nsapi_size_or_error_t bytes_sent = 0;

  printf("Sending message: \n%s", request);

  // Loop as long as there are more data to send
  while (bytes_to_send) {
    // Try to send the remaining data.
    // send() returns how many bytes were actually sent
    bytes_sent = socket->send(request + bytes_sent, bytes_to_send);

    if (bytes_sent < 0) {
      // Negative return values from send() are errors
      return bytes_sent;
    } else {
      printf("Sent %d bytes\n", bytes_sent);
    }

    bytes_to_send -= bytes_sent;
  }

  printf("Complete message sent\n");
  
  return bytes_to_send;
}

nsapi_size_or_error_t read_response(Socket *socket, char *buffer, int buffer_length) {

  if (socket == nullptr || buffer == nullptr || buffer_length < 1) {
    printf("Invalid function parameters\n");
    return NSAPI_ERROR_PARAMETER;
  }

  memset(buffer, 0, buffer_length);

  int remaining_bytes = buffer_length;
  int received_bytes = 0;

  // Loop as long as there are more data to read,
  // we might not read all in one call to recv()
  while (remaining_bytes > 0) {
    nsapi_size_or_error_t result =
        socket->recv(buffer + received_bytes, remaining_bytes);

    // If the result is 0 there are no more bytes to read
    if (result == 0) {
      break;
    }

    // Negative return values from recv() are errors
    if (result < 0) {
      return result;
    }

    received_bytes += result;
    remaining_bytes -= result;
  }

  printf("\nReceived %d bytes:\n%.*s\n", received_bytes,
         strstr(buffer, "\n") - buffer, buffer);

  return received_bytes;
}

