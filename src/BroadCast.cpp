/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "BroadCast.h"

#include <stdexcept>
#include <cstring>
#include <memory>

// blocking
#include <unistd.h>
#include <fcntl.h>
//#include <sys/select.h>
//#include <sys/socket.h>

#include <algorithm>
//#include <bits/errno.h>
//#include <csignal>

using namespace std;

UdpCatcher::UdpCatcher(uint16_t port) {
    this->sock.setBlocking(false);

    if (this->sock.bind(port) == sf::Socket::Status::Error) {
        throw runtime_error("UDP bind error");
    }
}

bool UdpCatcher::TryRecv(sf::IpAddress &from, int usecs) {
    sf::SocketSelector sel;
    sel.add(this->sock);

    if (!sel.wait(sf::microseconds(usecs)))
        return false;
    
    sf::Packet pack;
    uint16_t port;
    if (this->sock.receive(pack, from, port) == sf::Socket::Status::Error)
        throw runtime_error("Recvfrom error");
    return true;
}

void UdpCatcher::Close() {
    this->sock.unbind();
}

UdpSender::UdpSender(bool broadcast, uint16_t port) {
    this->broadcast = broadcast;
    this->port = port;
}

void UdpSender::Send(const sf::IpAddress &to) {
    sf::IpAddress t = to;

    if (this->broadcast)
        t = sf::IpAddress::Broadcast;

    sf::Packet pack;
    string message = "Searching!!!";
    pack.append(message.c_str(), message.length() + 1);

    while (true) {
        sf::Socket::Status s = this->sock.send(pack, t, this->port);
        if (s == sf::Socket::Status::Done)
            break;
        if (s == sf::Socket::Status::Error)
            throw runtime_error("Broadcasting error");
    }
}

void UdpSender::Close() {
}

TcpServer::TcpServer(uint16_t port, void (*newClient)(int id), void (*newMessage)(int id, const sf::Packet &pack), void (*oldClient)(int id)) {
    this->nextIndex = 0;
    this->HandleNewClient = newClient;
    this->HandleNewMessage = newMessage;
    this->HandleOldClient = oldClient;

    this->listening_sock.setBlocking(false);
    this->listening_sock.listen(port);
    
    if (this->listening_sock.listen(port) == sf::Socket::Status::Error)
        throw runtime_error("Socket listening");
}

int TcpServer::HandleNewEvents(int usecs) {
    struct timeval tim = {0, usecs};

    sf::SocketSelector sel;
    sel.add(this->listening_sock);
    for (auto &it: this->clients) {
        sel.add(*it.second);
    }

    if (!sel.wait(sf::microseconds(usecs)))
        return 0;

    if (sel.isReady(this->listening_sock)) {
        this->clients[this->nextIndex] = std::make_unique<sf::TcpSocket>();
        if (this->listening_sock.accept(*clients[this->nextIndex]) == sf::Socket::Status::Error)
            throw runtime_error("Socket accept");
        HandleNewClient(this->nextIndex++);
    }
    
    int count = 0;
    for (auto it = this->clients.begin(); it!= this->clients.end();) {
        if (sel.isReady(*it->second)) {
            count++;
            
            sf::Packet pack;
            sf::Socket::Status s = it->second->receive(pack);

            if (s == sf::Socket::Status::Error) {
//                fprintf(stderr, "Client %d socket error: %s\n", it->first, strerror(errno));
                fprintf(stderr, "Client %d socket error \n", it->first);
            } else if (s == sf::Socket::Status::Disconnected) {
                printf("client socket %d closed\n", it->first);
                it->second->disconnect();
            } else {
                HandleNewMessage(it->first, pack);
            }
            if (s == sf::Socket::Status::Error || s == sf::Socket::Status::Disconnected) {
                it->second->disconnect();
                HandleOldClient(it->first);
                it = this->clients.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
    return count;
}

void TcpServer::SendToAll(const sf::Packet &pack) {
    for (auto &it: this->clients) {
        SendToOne(it.first, pack);
    }
}

void TcpServer::SendToOne(int id, const sf::Packet &pack) {
    sf::Packet tmp;
    tmp.append(pack.getData(), pack.getDataSize());
    while (true) {
        sf::Socket::Status s = this->clients[id]->send(tmp);
        if (s == sf::Socket::Status::Done)
            break;
        if (s == sf::Socket::Status::Error)
            printf("client socket %d send error\n", id);
        printf("client socket %d sent not all. Retrying...\n", id);
    }
}

void TcpServer::Close() {
    for (auto &it: this->clients) {
//        shutdown(it.second, FD_)
        it.second->disconnect();
    }
    this->listening_sock.close();
}

TcpClient::TcpClient(const sf::IpAddress &server_address, uint16_t port, void (*newMessage)(const sf::Packet &pack)) {
//    memcpy(&this->server_address, server_address, sizeof(struct sockaddr_in));
    this->connected = false;
    this->connection_closed = false;
    this->HandleNewMessage = newMessage;

//    this->sock.setBlocking(false);
    sf::Socket::Status s = this->sock.connect(server_address, port, sf::milliseconds(500));

    if (s == sf::Socket::Status::Error) {
        perror("Client Connect!!! error");
        throw runtime_error("Client Connect!!! error");
    }
    
//    if (s == sf::Socket::Status::Done) {
//        connected = true;
//    } else {
////         connection attempt is in progress
//    }
    this->sock.setBlocking(false);
    connected = true;
}

bool TcpClient::HandleNewEvents(int usecs) {
//    struct timeval tim = {0, usecs};

    sf::SocketSelector sel;
    sel.add(sock);

    if (!sel.wait(sf::microseconds(usecs)))
        return false;

    sf::Packet pack;

    sf::Socket::Status s = sock.receive(pack);
    if (s == sf::Socket::Status::Error) {
        fprintf(stderr, "Socket error: %s\n", strerror(errno));
    } else if (s == sf::Socket::Status::Disconnected) {
//            printf("client socket %d closed", sock);
//            perror("closed?");
        sock.disconnect();
    } else {
        HandleNewMessage(pack);
    }
    if (s == sf::Socket::Status::Error || s == sf::Socket::Status::Disconnected) {
        this->connection_closed = true;
        sock.disconnect();
    }

    return true;
}


void TcpClient::Send(const sf::Packet &pack) {
    sf::Packet tmp;
    tmp.append(pack.getData(), pack.getDataSize());
    while (true) {
        sf::Socket::Status s = sock.send(tmp);
        if (s == sf::Socket::Status::Done)
            break;
        if (s == sf::Socket::Status::Error)
            printf("socket send error\n");
        printf("socket sent not all. Retrying...\n");
    }

}


bool TcpClient::Closed() {
    return connection_closed;
}


void TcpClient::Close() {
    sock.disconnect();
}