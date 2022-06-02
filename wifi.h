#ifndef __WIFI_H__
#define __WIFI_H__

#include "mbed.h"
#include "json.hpp"
#include "ipgeolocation_certificate.h"
#include "weatherapi_cert.h"
#define BLINKING_RATE1       10000ms

using json = nlohmann::json;


struct NewsStrings
{
    char headlineString[500];
};

nsapi_size_or_error_t send_request(Socket *socket, const char *request);

nsapi_size_or_error_t read_response(Socket *socket, char *buffer,
                                    int buffer_length);

const char *get_nsapi_error_string(nsapi_error_t err);
void parse_json_to_epoch(char *input, int &unix_time);
void connect_to_BBC(NetworkInterface *network, struct NewsStrings *strings);
void connect_to_WorldTime(NetworkInterface *network, int &unix_time);
void getLocation(NetworkInterface *network, std::string &latitude, std::string &longitude);
void getWeather(NetworkInterface *network, float &temperature, std::string &weatherDesc);
void testFunction();

//void connect_to_IpGeo();

#endif // __WIFI_H__
