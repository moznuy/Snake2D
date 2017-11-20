/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BroadCast.h
 * Author: lamar
 *
 * Created on November 7, 2017, 6:07 PM
 */

#ifndef BROADCAST_H
#define BROADCAST_H

#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>

// TODO: Close
class UdpCatcher {
private:
    int sock;
    uint16_t port;
    
public:
    UdpCatcher(uint16_t port);
    bool TryRecv(struct sockaddr_in *from, int usecs);
    void Close();
};

class UdpSender {
private:
    int sock;
    bool broadcast;
    uint16_t broadcast_port;
    
public:
    UdpSender(bool broadcast, uint16_t broadcast_port);
    void Send(struct sockaddr_in *to);
    void Close();
};

class TcpServer {
private:
    int listening_sock;
    std::map<int, int> clients;
    int nextIndex;
    
    void (*HandleNewClient)(int id);
    void (*HandleNewMessage)(int id, const char *message, size_t length);
    void (*HandleOldClient)(int id);
public:
    TcpServer(
        uint16_t port, 
        void (*newClient)(int id), 
        void (*newMessage)(int id, const char *message, size_t length),
        void (*oldClient)(int id)
    );
    void HandleNewEvents(int usecs);
    void SendToAll(const char *message, size_t length);
    void SendToOne(int id, const char *message, size_t length);
    void Close();
};

class TcpClient {
private:
//    struct sockaddr_in server_address;
    int sock;
    bool connected;
    bool connection_closed;
    
    void (*HandleNewMessage)(const char *message, size_t length);
public:
    TcpClient(
        struct sockaddr_in *server_address,
        void (*newMessage)(const char *message, size_t length)
    );
//    void TryConnect();
    bool HandleNewEvents(int usecs);
    void Send(const char *message, size_t length);
    bool Closed();
    void Close();
};

#endif /* BROADCAST_H */

