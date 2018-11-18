//
// Created by user on 11/17/2018.
//

#ifndef SNAKE2D_NETWORK_H
#define SNAKE2D_NETWORK_H

#ifdef WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define in_port_t u_short
    int inet_pton(int af, const char *src, in_addr* dst);

    #define LAST_ERROR WSAGetLastError()
    #define OPERATION_IN_PROGRESS WSAEWOULDBLOCK
    #define FOUND_ON_LOCALHOST WSAEADDRINUSE
#else
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    //#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
    #include <unistd.h> /* Needed for close() */
    #include <errno.h>
    #include <csignal>
    #include <fcntl.h>

    typedef int SOCKET;

    #define LAST_ERROR errno
    #define OPERATION_IN_PROGRESS EINPROGRESS
    #define FOUND_ON_LOCALHOST EADDRINUSE
#endif

int sockInit();
int sockQuit();
int sockShutDown(SOCKET sock);
int sockClose(SOCKET sock);
int setNonBlocking(SOCKET sock);

#endif //SNAKE2D_NETWORK_H
