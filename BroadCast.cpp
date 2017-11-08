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
#include <sys/socket.h>

#include <algorithm>

using namespace std;

UdpCatcher::UdpCatcher(uint16_t port) {
    this->port = port;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
        throw runtime_error("Socket creation");

    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
        throw runtime_error("Socket blocking");

    struct sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(this->port);
    if (bind(sock,(struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) 
        throw runtime_error("Socket binding");
}

bool UdpCatcher::TryRecv(struct sockaddr_in *from, int usecs) {
    struct timeval tim = {0, usecs};
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    
    int ret = select(sock + 1, &readfds, NULL, NULL, &tim);
    if (ret == -1)
        throw runtime_error("Select error");
    if (!FD_ISSET(sock, &readfds))
        return false;
    
    char buff[256];
    socklen_t len = sizeof(struct sockaddr_in);
    if (recvfrom(sock, buff, 256, 0, (struct sockaddr *)from, &len) < 0)
        throw runtime_error("Recvfrom error");
    return true;
}

void UdpCatcher::Close() {
    close(sock);
}

UdpSender::UdpSender(bool broadcast, uint16_t broadcast_port) {
    this->broadcast = broadcast;
    this->broadcast_port = broadcast_port;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
        throw runtime_error("Socket creation"); 
    
    if (broadcast) {
        int broadcastEnable = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) 
            throw runtime_error("Socket broadcasting option");
    }
}

void UdpSender::Send(sockaddr_in* to) {
    struct sockaddr_in s;
    if (this->broadcast) {
        memset(&s, 0, sizeof(struct sockaddr_in));
        s.sin_family = AF_INET;
        s.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    } else {
        memcpy(&s, to, sizeof(struct sockaddr_in));
    }
    s.sin_port = (in_port_t)htons(this->broadcast_port);
    
    char buffer[256];
    strncpy(buffer, "Searching!!!", 256);
    
    if (sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&s, sizeof(struct sockaddr_in)) < 0)
        throw runtime_error("Broadcasting error");
}

void UdpSender::Close() {
    close(sock);
}

TcpServer::TcpServer(uint16_t port, void (*newClient)(int id), void (*newMessage)(int id, char *message, size_t length), void (*oldClient)(int id)) {
    this->port = port;
    this->HandleNewClient = newClient;
    this->HandleNewMessage = newMessage;
    this->HandleOldClient = oldClient;
    
    listening_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_sock < 0) 
        throw runtime_error("Socket creation");

    if (fcntl(listening_sock, F_SETFL, O_NONBLOCK) < 0)
        throw runtime_error("Socket blocking");

    struct sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(this->port);
    if (bind(listening_sock,(struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) 
        throw runtime_error("Socket binding");
    
    if (listen(listening_sock, 5) < 0)
        throw runtime_error("Socket listening");
}

void TcpServer::HandleNewEvents(int usecs) {
    struct timeval tim = {0, usecs};
    
    fd_set readfds;
    FD_ZERO(&readfds);
    
    FD_SET(this->listening_sock, &readfds);
    for (auto &it: clients) {
        FD_SET(it, &readfds);
    }
    
    int nfds = this->listening_sock;
    for (auto &it:clients) {
        nfds = max(nfds, it);
    }
    
    int ret = select(++nfds, &readfds, NULL, NULL, &tim);
    if (ret == -1)
        throw runtime_error("Select error");
    
    if (FD_ISSET(this->listening_sock, &readfds)) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        int client = accept(this->listening_sock, (struct sockaddr *) &client_addr, &len);
        clients.push_back(client);
        
        HandleNewClient(clients.size() - 1);
    }
    
    for (int i=0; i < clients.size(); i++) {
        if (FD_ISSET(clients[i], &readfds)) {
            const int bufflen = 8192;
            char buff[bufflen];
            
            int ret = recv(clients[i], buff, bufflen, 0);
            if (ret < 0) {
                printf("error with socket %d", clients[i]);
            } else if (ret == 0) {
                printf("client socket %d closed", clients[i]);
                shutdown(clients[i], SHUT_RDWR);
            } else {
                HandleNewMessage(i, buff, ret);
            }
            if (ret <= 0) {
                close(clients[i]);
            }
            // REMOVE FROM VECTOR
        }
    }
    
//    if (!FD_ISSET(sock, &readfds))
//        return false;
//    
//    char buff[256];
//    socklen_t len = sizeof(struct sockaddr_in);
//    if (recvfrom(sock, buff, 256, 0, (struct sockaddr *)from, &len) < 0)
//        throw runtime_error("Recvfrom error");
//    return true;
}

void TcpServer::SendToAll(char *message, size_t length) {
    
}

void TcpServer::SendToOne(int id, char *message, size_t length) {
    
}
