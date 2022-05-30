#ifndef __WIFI_H__
#define __WIFI_H__

#include "mbed.h"

struct NewsStrings
{
    char firstString[200];
    char secondString[200];
    char thirdString[200];
};

nsapi_size_or_error_t send_request(Socket *socket, const char *request);

nsapi_size_or_error_t read_response(Socket *socket, char *buffer,
                                    int buffer_length);

void connect_to_BBC(struct NewsStrings *strings);

#endif // __WIFI_H__
