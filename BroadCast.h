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

#endif /* BROADCAST_H */

