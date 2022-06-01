#ifndef __WIFI_H__
#define __WIFI_H__

#include "mbed.h"
#include "ipify_org_ca_root_certificate.h"
#include "geolocation_cert.h"


struct NewsStrings
{
    char headlineString[500];
};

nsapi_size_or_error_t send_request(Socket *socket, const char *request);

nsapi_size_or_error_t read_response(Socket *socket, char *buffer,
                                    int buffer_length);

void connect_to_BBC(struct NewsStrings *strings);

void connect_to_IpGeo();

#endif // __WIFI_H__
