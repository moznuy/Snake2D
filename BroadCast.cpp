/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "BroadCast.h"

#include <stdexcept>
#include <cstring>
#include <algorithm>

using namespace std;

UdpCatcher::UdpCatcher(uint16_t port) {
    this->port = port;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
        throw runtime_error("Socket creation");

    if (setNonBlocking(sock) < 0)
        throw runtime_error("Socket blocking");

    struct sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(this->port);
    if (bind(sock,(struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) {
        perror("UDP bind error");
        throw runtime_error("UDP bind error");
    }
}

bool UdpCatcher::TryRecv(struct sockaddr_in *from, int usecs) {
    struct timeval tim = {0, usecs};
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    
    int ret = select(sock + 1, &readfds, NULL, NULL, &tim);
    if (ret == -1) {
        perror("UDP Select error");
        throw runtime_error("UDP Select error");
    }
    if (!FD_ISSET(sock, &readfds))
        return false;
    
    char buff[256];
    socklen_t len = sizeof(struct sockaddr_in);
    if (recvfrom(sock, buff, 256, 0, (struct sockaddr *)from, &len) < 0)
        throw runtime_error("Recvfrom error");
    return true;
}

void UdpCatcher::Close() {
    sockClose(sock);
}

UdpSender::UdpSender(bool broadcast, uint16_t broadcast_port) {
    this->broadcast = broadcast;
    this->broadcast_port = broadcast_port;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
        throw runtime_error("Socket creation"); 
    
    if (broadcast) {
        int broadcastEnable = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcastEnable, sizeof(broadcastEnable)) < 0)
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
    sockClose(sock);
}

TcpServer::TcpServer(uint16_t port, void (*newClient)(int id), void (*newMessage)(int id, const char *message, size_t length), void (*oldClient)(int id)) {
    this->nextIndex = 0;
    this->HandleNewClient = newClient;
    this->HandleNewMessage = newMessage;
    this->HandleOldClient = oldClient;
    
    listening_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_sock < 0) 
        throw runtime_error("Socket creation");

    if (setNonBlocking(listening_sock) < 0)
        throw runtime_error("Socket blocking");
    
    int yes = 1;
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(int)) < 0) {
        perror("Reuse addr");
        throw runtime_error("Reuse addr error");
    } 
    
    
    struct sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(listening_sock,(struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) {
        perror("TcpServer bind error");
        throw runtime_error("TcpServer bind error");
    }
    
    if (listen(listening_sock, 5) < 0)
        throw runtime_error("Socket listening");
}

int TcpServer::HandleNewEvents(int usecs) {
    struct timeval tim = {0, usecs};
    
    fd_set readfds;
    FD_ZERO(&readfds);
    
    FD_SET(this->listening_sock, &readfds);
    for (auto &it: clients) {
        FD_SET(it.second, &readfds);
    }
    
    SOCKET nfds = this->listening_sock;
    for (auto &it:clients) {
        nfds = max(nfds, it.second);
    }
    
    int ret = select(++nfds, &readfds, NULL, NULL, &tim);
    if (ret == -1) {
        perror("TcpServer Select error");
        throw runtime_error("TcpServer Select error");
    }
    
    if (FD_ISSET(this->listening_sock, &readfds)) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        SOCKET client = accept(this->listening_sock, (struct sockaddr *) &client_addr, &len);
        clients.insert(make_pair(this->nextIndex, client));
        
        HandleNewClient(this->nextIndex++);
    }
    
    int count = 0;
    for (auto it = clients.begin(); it != clients.end(); ) {
        if (FD_ISSET(it->second, &readfds)) {
            count++;
            
            const int bufflen = 8192;
            char buff[bufflen];
            
            ret = recv(it->second, buff, bufflen, 0);
            if (ret < 0) {
                fprintf(stderr, "Client %d socket error: %s\n", it->first, strerror(errno));
            } else if (ret == 0) {
                //printf("client socket %d closed\n", it->second);
                sockShutDown(it->second);
            } else {
                HandleNewMessage(it->first, buff, ret);
            }
            if (ret <= 0) {
                sockClose(it->second);
                HandleOldClient(it->first);
                it = clients.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
    return count;
}

void TcpServer::SendToAll(const char *message, size_t length) {
    for (auto &it: clients) {
        send(it.second, message, length, 0);
    }
}

void TcpServer::SendToOne(int id, const char *message, size_t length) {
    // TODO: IF
    send(clients.at(id), message, length, 0);
}

void TcpServer::Close() {
    for (auto &it: clients) {
//        shutdown(it.second, FD_)
        sockClose(it.second);
    }

    sockClose(this->listening_sock);
}

TcpClient::TcpClient(struct sockaddr_in* server_address, void(*newMessage)(const char*, size_t)) {
//    memcpy(&this->server_address, server_address, sizeof(struct sockaddr_in));
    this->connected = false;
    this->connection_closed = false;
    this->HandleNewMessage = newMessage;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
        throw runtime_error("Socket creation");

    if (setNonBlocking(sock) < 0)
        throw runtime_error("Socket blocking");
    
    int ret = connect(sock, (struct sockaddr *)server_address, sizeof(struct sockaddr_in));
    if (ret < 0 && errno != EINPROGRESS) {
        perror("Client Connect!!! error");
        throw runtime_error("Client Connect!!! error");
    }
    
    if (ret == 0) {
        connected = true;
    } else {
        // connection attempt is in progress
    }
}

bool TcpClient::HandleNewEvents(int usecs) {
    struct timeval tim = {0, usecs};
    
    fd_set readfds, writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    
    if (this->connected)
        FD_SET(sock, &readfds);
    if (!this->connected)
        FD_SET(sock, &writefds);
    
    int ret = select(sock + 1, &readfds, &writefds, NULL, &tim);
    if (ret == -1) {
        perror("Client Select error: ");
        throw runtime_error("Client Select error: ");
    }
    
    if (FD_ISSET(sock, &writefds)) {
        int val;
        socklen_t length = sizeof(int);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&val, &length) < 0)
            throw runtime_error("getsockopt error");
        
        if (val != 0) {
//            perror("Connection errno");
            if (errno != EINPROGRESS)
                throw runtime_error("connection error");
            else 
                return false;
//                throw runtime_error("must never happen. maybe?");
        }
        
        connected = true;
        return true;
    }
    
    if (FD_ISSET(sock, &readfds)) {
        const int bufflen = 8192;
        char buff[bufflen];

        int ret = recv(sock, buff, bufflen, 0);
        if (ret < 0) {
            fprintf(stderr, "Socket error: %s\n", strerror(errno));
        } else if (ret == 0) {
//            printf("client socket %d closed", sock);
//            perror("closed?");
            sockShutDown(sock);
        } else {
            HandleNewMessage(buff, ret);
        }
        if (ret <= 0) {
            this->connection_closed = true;
            sockClose(sock);
        }
        
        return true;
    }
    return false;
}


void TcpClient::Send(const char *message, size_t length) {
    // TODO: IF
    send(sock, message, length, 0);
}


bool TcpClient::Closed() {
    return connection_closed;
}


void TcpClient::Close() {
    sockClose(sock);
}