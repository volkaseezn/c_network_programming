/*
* Checking if this changes anything.
*/
#include "chp3.h"
#include <ctype.h>



int main() {

// initialising winsock for windows
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)){
        fprintf(stderr, "Failed to initialise.\n");
        return 1;
    }
#endif

    printf("Configuring local address...\n");
    // defining the hints to be used by getaddrinfo()
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // defining where to store the found addresses according to hints.
    struct addrinfo *bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address);

    printf("Creating socket...\n");
    // creating the socket endpoint for the server
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    // checking that the socket was properly created.
    if (!ISVALIDSOCKET(socket_listen)){
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");
    // binding the socket to a local address (giving the socket a name), while checking that the snippet run successfully.
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)){
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    // free memory allocated for the dynamically allocated linked list
    freeaddrinfo(bind_address);

    printf("Listening...\n");
    // setting the socket to LISTEN state
    if (listen(socket_listen, 10) < 0){
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    // setting fdset to store active sockets
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    // max_socket is the only socket at this point
    SOCKET max_socket = socket_listen;

    printf("Waiting for connections...\n");

    // main program loop (server runs infintely)
    while(1){
        // copying the fdset as select() modifies in-place
        fd_set reads;
        reads = master;
        // checking that there is no error
        if (select(max_socket+1, &reads, 0, 0, 0) < 0){
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }
        // checking all possible entries in fdset from 1 to max_socket
        SOCKET i;
        for (i = 1; i <= max_socket; ++i){
            // checking that the current socket fd has been flagged by select
            if (FD_ISSET(i, &reads)){
                // checking that the flagged socket fd is that of the client. It means it is ready to accept()
                // For other ready sockets, it would mean data is ready to be read using recv()
                if (i == socket_listen){
                    // to create and store the new accepted connection request.
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    // to store the accepted connection fd
                    SOCKET socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
                    // checking that the socket is valid
                    if (!ISVALIDSOCKET(socket_client)){
                        fprintf(stderr, "accept() failed. (%d)", GETSOCKETERRNO());
                        return 1;
                    }

                    // adding new connection's socket to master fdset and setting new max_socket
                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    // to store the address name info
                    char address_buffer[100];
                    // getting the address name info and printing it in human readable format
                    getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);
                }        // the case for established connection.
                else {
                    // to store the received data
                    char read[1024];
                    // to store the number of bytes received, and place the number of 
                    int bytes_received = recv(i, read, 1024, 0);
                    // checking that it received correctly, and if not, removing the socket from the master fdset and close it
                    if (bytes_received < 1){
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }

                    // performing the toupper() microservice operation
                    int j;
                    for(j = 0; j < bytes_received; ++j)
                        read[j] = toupper(read[j]);
                    
                    // sending the formatted string back to the socket we are listening on
                    send(i, read, bytes_received, 0);
                }
            }
        }
    }

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

#if defined(_WIN32)
    WSACleanup();
#endif

    printf("Finished.\n");
    return 0;    

}