#include "wifi.h"
#include "mbed.h"

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

nsapi_size_or_error_t read_response(Socket *socket, char *buffer,
                                    int buffer_length) {

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

void connect_to_BBC(char temp[])
{
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

    const char *host = "feeds.bbci.co.uk";
    //const char *host = "example.org";
    result = network->gethostbyname(host, &address);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of host %s: %d\n", host, result);
        while(1);
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());

    address.set_port(80);

    result = socket->connect(address);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to server at %s: %d\n", host, result);
        while(1);
    }

    printf("Successfully connected to server %s\n", host);

    const char request[] = "GET /news/world/rss.xml# HTTP/1.1\r\n"
                           "Host: feeds.bbci.co.uk\r\n"
                           "Connection: close\r\n"
                           "\r\n";

    result = send_request(socket, request);


    static constexpr size_t HTTP_RESPONSE_BUF_SIZE = 20000;
    static char response[HTTP_RESPONSE_BUF_SIZE] = { 0 };
    int remaining_bytes = HTTP_RESPONSE_BUF_SIZE;
    int received_bytes = 0;

    result = read_response(socket, response, HTTP_RESPONSE_BUF_SIZE);

    if(result < 0)
    {
        printf("Failed to read response: %d\n", result);
    }

    response[result] = '\0';
    //printf("\nThe HTTP GET response:\n%s\n", response);
    strcpy(temp, response);
}