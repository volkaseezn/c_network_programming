#include "chp6.h"

#define TIMEOUT 5.0

// helper function to make sense of the provided url
void parse_url(char *url, char **hostname, char **port, char **path){
    printf("URL: %s\n", url);
    // to declare a character pointer
    char *p;
    // searching for :// in the url and storing it in pointer
    p = strstr(url, "://");
    // setting null pointer for protocol since nothing has been found yet
    char *protocol = 0;
    // checking that an occurence was found
    if (p){
        // point protocol to beginning of the url
        protocol = url;
        // setting the character pointer to null
        *p = 0;
        // pointing to one after :// 
        p += 3;
    }else{
        // point to the beginning of the url
        p = url;
    }
    if (protocol){
        if (strcmp(protocol, "http")){
            fprintf(stderr, "Unknown protocol '%s'. Only 'http' is supported.\n", protocol);
            exit(1);
        }
        *hostname = p;
        // looping until pointer gets to the first colon, slash, or hash
        while (*p && *p != ':' && *p != '/' && *p != '#') ++p;
        // setting the port to '80', except a port number has been specified as checked in the subsequent if statement
        *port = "80";
        if (*p == ':'){
            // replace colon with null termination then point to the next character
            *p++ = 0;
            // return the found port's address to port
            *port = p;
        }
        while (*p && *p != '/' && *p != '#') ++p;
        // url path starts here
        *path = p;
        // checking to find the first slash, which will be ignored since all hostnames start with a slash
        if (*p == '/'){
            *path = p + 1;
        }
        *p = 0;
        while (*p && *p != '#') ++p;
        if (*p == '#') *p = 0;
    }
    printf("hostname: %s\n", hostname);
    printf("port: %s\n", port);
    printf("path :%s\n", path);
}

// function to send the http request to server
void send_request(SOCKET s, char *hostname, char *port, char *path){
    // storing the http request info
    char buffer[2048];
    sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
    // this pointer arithmetic is necessary to get the data sent to the right location in memory
    sprintf(buffer + strlen(buffer), "Host: %s:%s\n", hostname, port);
    sprintf(buffer + strlen(buffer), "Connection: close\r\n");
    sprintf(buffer + strlen(buffer), "User-Agent: honpwc web_get 1.0\r\n");
    sprintf(buffer + strlen(buffer), "\r\n");

    send(s, buffer, strlen(buffer), 0);
    printf("Sent Headers:\n%s", buffer);
}

// helper function to attempt connecting to host through which to send the http request
SOCKET connect_to_host(char *hostname, char *port){
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(hostname, port, &hints, *peer_address)){
        fprintf(stderr, "getaddrinfo () failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }
    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);
}
