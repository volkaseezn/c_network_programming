#include "chp5.h"


const unsigned char *print_name(const unsigned char *msg, const unsigned char *p, const unsigned char *end){
    if (p + 2 > end){
        fprintf(stderr, "End of message.\n");
        exit(1);
    }
    // checking if the 
    if ((*p & 0xC0) == 0xC0){
        // performing a bitwise AND operation on the value stored in p with mask 0x3F, then left shifting by 8 bits, and adding the value stored in p
        const int k = ((*p & 0x3F) << 8) + p[1];
        // pointer arithmetic shifting p by 2
        p += 2;
        printf(" (pointer %d) ", k);
        // recursively calling print_name if there are still more pointers
        print_name(msg, msg+k, end);
        return p;
    } else{
        // dereferencing p and storing the value in len (length of the name)
        const int len = *p++;
        // checking if the length is not beyond the end of the message
        if (p + len + 1 > end){
            fprintf(stderr, "End of message.\n");
            exit(1);
        }
        // printing the next len characters to the console
        printf("%.*s", len, p);
        p += len;
        if (*p){
            printf(".");
            return print_name(msg, p, end);
        }else{
            return p+1;
        }
        
    }
}

void print_dns_message(const char *message, int msg_length){
    if (msg_length < 12){
        fprintf(stderr, "Message is too short to be valid.\n");
        exit(1);
    }
    // copying message pointer into a new variable and type casting to allow for easier calculations
    const unsigned char *msg = (const unsigned char *)message;
    // to print the dns message line by line
    // int i;
    // for (i = 0; i < msg_length; ++i){
    //     unsigned char r = msg[i];
    //     printf("02d: 02X 03d '%c'\n", i, r, r, r);
    // }
    // printf("\n");

    // READING THE HEADER

    // printing the id (first 2 bytes of the message)
    printf("ID = %0X %0X\n", msg[0], msg[1]);
    // read the QR bit of the message to see if it is set (response)
    const int qr = (msg[2] & 0x80) >> 7;
    printf("QR = %d %s\n", qr, qr ? "response" : "query");

    //printing the OPCODE (next 4 bits)
    const int opcode = (msg[2] & 0x78 >> 3);
    printf("OPCODE = %d", opcode);
    // checking what the value of the opcode is and printing the meaning
    switch(opcode){
        case 0: printf("standard\n"); break;
        case 1: printf("reverse\n"); break;
        case 2: printf("status\n"); break;
        default: printf("?\n"); break;
    }

    const int aa = (msg[2] & 0x04) >> 2;
    printf("AA = %d %s\n", aa, aa ? "authoritative" : "");

    const int tc = (msg[2] & 0x02) >> 1;
    printf("TC = %d %s\n", tc, tc ? "message truncated" : "");

    const int rd = (msg[2] & 0x01);
    printf("RD = %d %s\n", rd, rd ? "recursion desired" : "");

    //checking for rcode in 3rd byte of message
    if (qr){
        const int rcode = msg[3] & 0x0F;
        printf("RCODE = %d ", rcode);
        switch(rcode){
            case 0: printf("success\n"); break;
            case 1: printf("format error\n"); break;
            case 2: printf("server failure`n"); break;
            case 3: printf("name error\n"); break;
            case 4: printf("not implemented\n"); break;
            case 5: printf("refused\n"); break;
            default: printf("?\n"); break;
        }
        if (rcode != 0) return;
    }

    // reading the next pieces of information in the header. The bitwise shift is performed to properly make this a 16 bit value 
    const int qdcount = (msg[4] << 8) + msg[5];
    const int ancount = (msg[6] << 8) + msg[7];
    const int nscount = (msg[8] << 8) + msg[9];
    const int arcount = (msg[10] << 8) + msg[11];

    printf("QDCOUNT = %d\n", qdcount);
    printf("ANCOUNT = %d\n", ancount);
    printf("NSCOUNT = %d\n", nscount);
    printf("ARCOUNT = %d\n", arcount);

    const unsigned char *p = msg + 12;
    const unsigned char *end = msg + msg_length;

    // reading and printing every question in the dns message
    if (qdcount){
        int i;
        for (i = 0; i < qdcount; ++i){
            if (p >= end){
                fprintf(stderr, "End of message.\n"); exit(1);
            }
            printf("Query %2d\n", i + 1);
            printf("   name: ");
            
            p = print_name(msg, p, end);
            printf("\n");

            if (p + 4 > end){
                fprintf(stderr, "End of message.\n"); exit(1);
            }
            // printing the question type
            const int type = (p[0] << 8) + p[1];
            printf("   type: %d\n", type);
            // shifting the pointer 2 bytes forward
            p += 2;
            // printing the question class
            const int qclass = (p[0] << 8) + p[1];
            printf("   class: %d\n", qclass);
            p += 2;
        }
    }
    if (ancount || nscount || arcount){
        int i;
        for (i = 0; i < ancount + nscount + arcount; ++i){
            if (p >= end){
                fprintf(stderr, "End of message.\n"); exit(1);
            }
            printf("Answer: %2d\n", i + 1);
            printf("    name: ");

            p = print_name(msg, p, end); printf("\n");

            if (p + 10 > end){
                fprintf(stderr, "End of message.\n"); exit(1);
            }
            const int type = (p[0] << 8) + p[1];
            printf("    type: %d\n", type);
            p += 2;

            const int qclass = (p[0] << 8) + p[1];
            printf("    class: %d\n", qclass);
            p += 2;
            // storing ttl value by shifting each byte of the message to the corresponding position in this 32bit integer
            const unsigned int ttl = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
            printf("    ttl: %u\n", ttl);
            p += 4;

            const int rdlen = (p[0] << 8) + p[1];
            printf("    rdlen: %d\n", rdlen);
            p += 2;

            if (p + rdlen > end){
                fprintf(stderr, "End of message.\n"); exit(1);
            }
            // reading answer data
            if (rdlen == 4 && type == 1){ /*A Record*/
                printf("Address ");
                printf("%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
            }else if (type == 15 && rdlen > 3){ /* MX Record*/
                const int preference = (p[0] << 8) + p[1];
                printf("    pref: %d\n", preference);
                printf("MX: ");
                print_name(msg, p+2, end); printf("\n");
            }else if (rdlen == 16 && type == 28){ /* AAAA Record*/
                printf("Address: ");
                int j;
                for (j = 0; j < rdlen; j += 2){
                    printf("%02x%02x", p[j], p[j+1]);
                    if (j+2 < rdlen) printf(":");
                }
                printf("\n");
            }else if (type == 16){ /* TXT Record */
                printf("TXT: %.*s\n", rdlen-1, p+1);
            }else if (type == 5){
                printf("CNAME: ");
                print_name(msg, p, end); printf("\n");
            }
            // pushing pointer to the end of the answer
            p += rdlen;
        }
    }
    if (p != end){
        printf("There is some unread data left over.\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]){
    // checking proper usage of the program
    if (argc < 3){
        printf("Usage:\n\tdns_query2 hostname type\n");
        printf("Example:\n\tdns_query2 example.com aaaa\n");
        exit(0);
    }
    // checking that the hostname is not too long
    if (strlen(argv[1]) > 255){
        fprintf(stderr, "Hostname too long.");
        exit(1);
    }
    // to store the record type specified by the caller
    unsigned char type;
    //checking which record type was specified
    if (strcmp(argv[2], "a") == 0){
        type = 1;
    }else if (strcmp(argv[2], "mx") == 0){
        type = 15;
    }else if (strcmp(argv[2], "txt") == 0){
        type = 16;
    }else if (strcmp(argv[2], "aaaa") == 0){
        type = 28;
    }else if (strcmp(argv[2], "any") == 0){
        type = 255;
    }else{
        fprintf(stderr, "Unknown type '%s'. Use a, aaaa, txt, mx, or any.", argv[2]);
        exit(1);
    }
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)){
        fprintf(stderr, "Failed to initialise.\n");
        return 1;
    }
#endif
    // resolving 
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo *peer_address;
    if (getaddrinfo("8.8.8.8", "53", &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    // creating socket
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)){
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    // constructing data for dns query (header)
    char query[1024] = {0xAB, 0xCD, /* ID */
                        0x01, 0x00, /* Set recursion */
                        0x00, 0x01, /* QDCOUNT */
                        0x00, 0x00, /* ANCOUNT */
                        0x00, 0x00, /* NSCOUNT */
                        0x00, 0x00 /* ARCOUNT */

    };
    // (question)
    char *p = query + 12;
    char *h = argv[1];

    while(*h){
        char *len = p;
        p++;
        if (h != argv[1]){
            ++h;
        }
        // looping as long as character pointed to by h is not a null terminator or dot.
        while (*h && *h != '.'){
            // copies characters pointed to by h to p, and increments them
            *p++ = *h++;
        }
        *len = p - len - 1;
    }
    // adds a null terminator at the end of the query array.
    *p++ = 0;
    // (additon of question type)
    *p++ = 0x00; *p++ = type;
    // (addition of question class)
    *p++ = 0x00; *p++ = 0x01;
    // indicating the query size after all parts have been added
    const int query_size = p - query;
    int bytes_sent = sendto(socket_peer, query, query_size, 0, peer_address->ai_addr, peer_address->ai_addrlen);
    printf("Sent %d bytes.\n", bytes_sent);

    print_dns_message(query, query_size);

    char read[1024];
    int bytes_received = recvfrom(socket_peer, read, 1024, 0, 0, 0);
    printf("Received %d bytes.\n", bytes_received);
    print_dns_message(read, bytes_received);
    printf("\n");

    freeaddrinfo(peer_address);
    CLOSESOCKET(socket_peer);

#if defined(_WIN32)
    WSACleanup();
#endif
    return 0;
}