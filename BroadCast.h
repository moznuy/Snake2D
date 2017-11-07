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

const int response_port = 5689;
const int broadcast_port = 5688;

class BroadcastSender {
private:
//    int response_socket;
public:
//    Broadcast();
    void SendBroadcast();
};

class BroadcastCatcher {
private:
    int sock;
public:
    BroadcastCatcher();
    bool RecvBroadcast(struct sockaddr_in *from);
};

#endif /* BROADCAST_H */

