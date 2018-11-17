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
#include <map>
#include <memory>
#include <SFML/Network.hpp>

// TODO: Close
class UdpCatcher {
private:
    sf::UdpSocket sock;

public:
    UdpCatcher(uint16_t port);
    bool TryRecv(sf::IpAddress &from, int usecs);
    void Close();
};

class UdpSender {
private:
    sf::UdpSocket sock;
    bool broadcast;
    uint16_t port;
    
public:
    UdpSender(bool broadcast, uint16_t port);
    void Send(const sf::IpAddress &to);
    void Close();
};

class TcpServer {
private:
    sf::TcpListener listening_sock;
    std::map<int, std::unique_ptr<sf::TcpSocket>> clients;
    int nextIndex;
    
    void (*HandleNewClient)(int id);
    void (*HandleNewMessage)(int id, const sf::Packet &pack);
    void (*HandleOldClient)(int id);
public:
    TcpServer(
        uint16_t port, 
        void (*newClient)(int id), 
        void (*newMessage)(int id, const sf::Packet &pack),
        void (*oldClient)(int id)
    );
    int HandleNewEvents(int usecs);
    void SendToAll(const sf::Packet &pack);
    void SendToOne(int id, const sf::Packet &pack);
    void Close();
};

class TcpClient {
private:
    sf::TcpSocket sock;
    bool connected;
    bool connection_closed;
    
    void (*HandleNewMessage)(const sf::Packet &pack);
public:
    TcpClient(
        const sf::IpAddress &server_address, uint16_t port,
        void (*newMessage)(const sf::Packet &pack)
    );
    bool HandleNewEvents(int usecs);
    void Send(const sf::Packet &pack);
    bool Closed();
    void Close();
};

#endif /* BROADCAST_H */

