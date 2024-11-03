#include "chp4.h"

int main(){

#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)){
        fprintf(stderr, "Failed to initialise.\n");
        return 1;
    }
#endif

    printf("Configuring remote addres...\n");
    // to store the address hints info
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    // only the socket type needs to be set
    hints.ai_socktype = SOCK_DGRAM;
    // to store matching address
    struct addrinfo *peer_address;
    // we manually specify the host and port 
    if (getaddrinfo("127.0.0.1", "8080", &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    // converting the matched address to human readable format
    printf("Remote address is: ");
    // to store the host
    char host[100];
    // to store the port
    char service[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);
    printf("%s %s\n", host, service);

    printf("Creating socket...\n");
    // to store the fd of the generated socket for the peer
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)){
        fprintf(stderr, "socket() failed. (%d)", GETSOCKETERRNO());
        return 1;
    }
    // NOTE: we do not bind here as the (ephemeral) local port used here is not important in this case.

    // holding the message to send to the peer server
    const char *message = "No, I won't say hello to the world.";
    // sends the aforementioned message to the specified socket and returns the amount of bytes sent
    int bytes_sent = sendto(socket_peer, message, strlen(message), 0, peer_address->ai_addr, peer_address->ai_addrlen);
    printf("Sent %d bytes.\n", bytes_sent);

    // freeing address held up by the matchihng addresses structure
    freeaddrinfo(peer_address);
    CLOSESOCKET(socket_peer);

#if defined(_WIN32)
    WSACleanup();
#endif

    printf("Finished.\n");
    return 0;
}