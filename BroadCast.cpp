/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "BroadCast.h"

#include <stdexcept>
#include <netinet/in.h>
#include <cstring>

// blocking
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

using namespace std;

BroadcastCatcher::BroadcastCatcher() {
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) 
       throw runtime_error("Socket creation");
   
   if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
       throw runtime_error("Socket blocking");

   struct sockaddr_in server;
   memset(&server, 0, sizeof(sockaddr_in));
   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(broadcast_port);
   if (bind(sock,(struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) 
       throw runtime_error("Socket binding");
}

bool BroadcastCatcher::RecvBroadcast(struct sockaddr_in *from) {
    struct timeval tim = {0, 1000};
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    
    int ret = select(sock + 1, &readfds, NULL, NULL, &tim);
    if (ret == -1)
        throw runtime_error("Select error");
    if (ret == 0)
        return false;
    
    char buff[256];
    socklen_t len = sizeof(struct sockaddr_in);
    if (recvfrom(sock, buff, 256, 0, (struct sockaddr *)from, &len) < 0)
        throw runtime_error("Recvfrom error");
    return true;
}

void BroadcastSender::SendBroadcast() {
    int sock, n;
    char buffer[256];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
        throw runtime_error("Socket creation");

    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) 
        throw runtime_error("Socket broadcasting option");
   
   
    struct sockaddr_in s;
    memset(&s, 0, sizeof(struct sockaddr_in));
    s.sin_family = AF_INET;
    s.sin_port = (in_port_t)htons(broadcast_port);
    s.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    strncpy(buffer, "Searching!!!", 256);
    
    if (sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&s, sizeof(struct sockaddr_in)) < 0)
        throw runtime_error("Broadcasting error");
}