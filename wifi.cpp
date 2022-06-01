#include "wifi.h"

using json = nlohmann::json;









nsapi_size_or_error_t send_request(Socket *socket, const char *request)
{
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









void parse_json_to_epoch(char *input, int &unix_time)
{
    json j_object = json::parse(input, nullptr, false);

    if (j_object.is_discarded())
    {
        printf("The input is invalid JSON\n");
        return;
    }
    printf("The input is valid JSON\n");

    unix_time = j_object["unixtime"].get<int>();
}









void connect_to_BBC(NetworkInterface *network, struct NewsStrings *pNews)
{
    /*---News Feed Get Request---*/
    nsapi_size_or_error_t result;

    do
    {
        printf("\nConnecting to network for BBC...\n");
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

    // Using BBCs RSS feed
    const char *host = "feeds.bbci.co.uk";
    result = network->gethostbyname(host, &address);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of host %s: %d\n", host, result);
        while(true);
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());

    address.set_port(80);

    result = socket->connect(address);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to server at %s: %d\n", host, result);
        while(true);
    }

    printf("Successfully connected to server %s\n", host);

    // Gets the exact website to extract XML
    const char request[] = "GET /news/world/rss.xml# HTTP/1.1\r\n"
                           "Host: feeds.bbci.co.uk\r\n"
                           "Connection: close\r\n"
                           "\r\n";

    result = send_request(socket, request);

    // Only allocate 4000 bytes because we only need the first part of the response
    // AKA the first three headlines and dont need to give more size to the string
    static constexpr size_t HTTP_RESPONSE_BUF_SIZE = 4000;
    char response[HTTP_RESPONSE_BUF_SIZE] = { 0 };
    int remaining_bytes = HTTP_RESPONSE_BUF_SIZE;
    int received_bytes = 0;

    result = read_response(socket, response, HTTP_RESPONSE_BUF_SIZE);
    socket->close();

    delete socket;
    socket = nullptr;

    network->disconnect();


    if(result < 0)
    {
        printf("Failed to read response: %d\n", result);
    }

    response[result] = '\0';
   
    char *temp = std::move(response);

/*--- Making three strings out of the XML response and storing them in the struct ---*/
    char firstString[200] = { 0 };
    char secondString[200] = { 0 };
    char thirdString[200] = { 0 };
    char startingItem[] = "<item>";
    char closingItem[] = "</item>";
    char startingWrap[] = "<title><![CDATA[";
    char closingWrap[] = "]]></title>";


    temp = strstr(temp, startingItem);
    temp = strstr(temp, startingWrap);
    temp = temp + strlen(startingWrap);

    int lineLength = 0;
    while(temp[lineLength] != '\n')
        lineLength++;

    strncpy(firstString, temp, lineLength);

    firstString[strlen(firstString) - strlen(closingWrap)] = '\0';


// Second string
    temp = strstr(temp, closingItem);
    temp = strstr(temp, startingItem);
    temp = strstr(temp, startingWrap);
    temp = temp + strlen(startingWrap);

    lineLength = 0;
    while(temp[lineLength] != '\n')
        lineLength++;

    strncpy(secondString, temp, lineLength);
    secondString[strlen(secondString) - strlen(closingWrap)] = '\0';


// Third string
    temp = strstr(temp, closingItem);
    temp = strstr(temp, startingItem);
    temp = strstr(temp, startingWrap);
    temp = temp + strlen(startingWrap);

    lineLength = 0;
    while(temp[lineLength] != '\n')
        lineLength++;

    strncpy(thirdString, temp, lineLength);
    thirdString[strlen(thirdString) - strlen(closingWrap)] = '\0';


// Composing three strings into one
    strcat(pNews->headlineString, firstString);
    for (int i = 0; i < 16; i++)
        strcat(pNews->headlineString, " ");
    strcat(pNews->headlineString, secondString);
    for (int i = 0; i < 16; i++)
        strcat(pNews->headlineString, " ");
    strcat(pNews->headlineString, thirdString);
} 









void connect_to_WorldTime(NetworkInterface *network, int &unix_time)
{
    nsapi_size_or_error_t result;
    do
    {
        printf("\nConnecting to network for World Time...\n");
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

    // Connecting to worldtimeapi with public ip address to get unix time   
    const char *host = "worldtimeapi.org";
    result = network->gethostbyname(host, &address);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to get IP address of host %s: %d\n", host, result);
        while(true);
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());

    address.set_port(80);

    result = socket->connect(address);

    if(result != NSAPI_ERROR_OK)
    {
        printf("Failed to connect to server at %s: %d\n", host, result);
        while(true);
    }

    printf("Successfully connected to server %s\n", host);

    // Get the timezone for oslo (Grimstads timezone as well)
    const char request[] = "GET /api/ip HTTP/1.1\r\n"
                           "Host: worldtimeapi.org\r\n"
                           "Connection: close\r\n"
                           "\r\n";

    result = send_request(socket, request);
    if(result != 0)
    {
        printf("Failed to send request: %d\n", result);
        socket->close();
    }

    // Receive data
    static constexpr size_t HTTP_RESPONSE_BUF_SIZE = 4000;
    static char http_response[HTTP_RESPONSE_BUF_SIZE] = { 0 };
    int remaining_bytes = HTTP_RESPONSE_BUF_SIZE;
    int received_bytes = 0;

    result = read_response(socket, http_response, HTTP_RESPONSE_BUF_SIZE);

    // Make a string out of only the json part of the response
    char *json_start = strchr(http_response, '{');

    parse_json_to_epoch(json_start, unix_time);

    printf("Unix time: %d\n", unix_time);

    socket->close();
    delete socket;
    socket = nullptr;

    network->disconnect();
}
