#include "chp5.h"

#ifndef AI_ALL
#define AI_ALL 0x0100
#endif

int main(int argc, char *argv[]){
    
    if (argc < 2){
        printf("Usage:\n\tlookup hostname\n");
        printf("Example:\n\tlookup example.com\n");
        exit(0);
    }

#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)){
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif

    printf("Resolving hostname '%s'\n", argv[1]);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_ALL;
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], 0, &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Remote address is: \n");
    // storing peer_address into a new variable
    struct addrinfo *address = peer_address;
    // entering a loop to print all addresses
    do {
        // buffer to to store text address
        char address_buffer[100];
        // to fill in the address. NI_NUMERICHOST is to get ip address and not hostname
        getnameinfo(address->ai_addr, address->ai_addrlen, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
        printf("\t%s\n", address_buffer);
    }// walking through the linked list
    while ((address = address->ai_next));

    freeaddrinfo(peer_address);

#if defined(_WIN32)
    WSACleanup();
#endif

    return 0;

}