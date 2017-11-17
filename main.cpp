/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: lamar
 *
 * Created on November 7, 2017, 2:44 PM
 */

// scp ./dist/Debug/GNU-Linux/snake2d sberko@10.70.80.96:/home/sberko/Tmp/snake2d

#include <cstdlib>
#include <bits/stdc++.h>
#include <SDL2/SDL.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "BroadCast.h"

using namespace std;

enum direction {
    None,
    North,
    East,
    South,
    West,
};

map<direction, SDL_Point> delta = {
    {None,  {0,  0}}, 
    {North, {0, -1}}, 
    {East,  {1,  0}},
    {South, {0,  1}},
    {West,  {-1, 0}},
};

SDL_Point operator+(const SDL_Point &a, const SDL_Point &b) {
    return {a.x + b.x, a.y + b.y};
}

bool operator==(const SDL_Point &a, const SDL_Point &b) {
    return a.x == b.x && a.y == b.y;
}

//typedef uint8_t crc;
//#define WIDTH  (8 * sizeof(crc))
//#define TOPBIT (1 << (WIDTH - 1))
//#define POLYNOMIAL 0xD7
//
//crc  crcTable[256];
//
//void crcInit(void) {
//    crc  remainder;
//    for (int dividend = 0; dividend < 256; ++dividend) {
//        remainder = dividend << (WIDTH - 8);
//
//        for (uint8_t bit = 8; bit > 0; --bit) {		
//            if (remainder & TOPBIT)
//                remainder = (remainder << 1) ^ POLYNOMIAL;
//            else
//                remainder = (remainder << 1);
//        }
//        crcTable[dividend] = remainder;
//    }
//} 
//
//crc crcFast(const uint8_t *message, int nBytes)
//{
//    uint8_t data;
//    crc remainder = 0;
//
//    for (int byte = 0; byte < nBytes; ++byte)
//    {
//        data = message[byte] ^ (remainder >> (WIDTH - 8));
//        remainder = crcTable[data] ^ (remainder << 8);
//    }
//    return (remainder);
//}

typedef unsigned long long checksum;
checksum getSum(const char *message, size_t length) {
    checksum res = 0;
    for (size_t i = 0; i < length; i++) {
        res += message[i];
    }
    return res;
}

typedef int sizeType;
const int sizeOfSize = sizeof(sizeType);

class Stream {
    char *stream;
    size_t capacity;
    size_t pos;
    
    bool own;
    
    void SetSize(sizeType size) {
        if (!own)
            throw runtime_error("Stream is not mine");
        
        *(sizeType *)stream = size;
    }
    
    void Dispose(bool completly) {
        if (!own)
            return;
        
        if (stream != nullptr) {
            delete[] stream;
        }
        if (completly) {
            stream = nullptr;
        } else {
            stream = new char[sizeOfSize];
            capacity = sizeOfSize;
            Clear();
        }
    }
    
    void Reserve(size_t newCapacity) {
        if (!own)
            throw runtime_error("Stream is not mine");
        
        if (newCapacity > this->capacity) {
            char *nw = new char[newCapacity];
            if (stream != nullptr) {
                memcpy(nw, stream, GetSize());
                delete[] stream;
            }
            stream = nw;
            this->capacity = newCapacity;
        }
    }
public:
    Stream() {
        stream = new char[sizeOfSize];
        capacity = sizeOfSize;
        
        own = true;
        Clear();
    }
    Stream(char *data, size_t length) {
        stream = data;
        capacity = length;
        pos = sizeOfSize;
        
        own = false;
    }
    
    sizeType GetSize() const {
        return *(sizeType *)stream;
    }
    
    void Clear() {
        if (!own)
            throw runtime_error("Stream is not mine");
        
        SetSize(sizeOfSize);
        ResetRead();
    }
    
    
    
    virtual ~Stream() {
        Dispose(true);
    }

    

    
    template<typename T>
    void Push(const T &info) {
        if (!own)
            throw runtime_error("Stream is not mine");
        
        auto size = GetSize();
        if (size + sizeof(T) > capacity) {
            Reserve(max(sizeType(size * 2), sizeType(size + sizeof(T))));
        }
        memcpy(stream + size, &info, sizeof(T));
        size += sizeof(T);
        SetSize(size);
    }
    
//    void AddCrc() {
//        checksum check = getSum(stream, size);  //(crc *)stream, size);
//        Push(check);
//    }
//    
//    bool ChechCrc() {
//        if (size < sizeof(checksum))
//            throw runtime_error("nope");
//        size -= sizeof(checksum);
//        checksum check = getSum(stream, size);  //crcFast((crc *)stream, size);
//        return check == *(checksum *)(stream + size);
//    }
    
    void ResetRead() {
        pos = sizeOfSize;
    }
    
    template<typename T>
    void Pull(T &info) {
        if (sizeof(T) + pos > GetSize()) {
            throw runtime_error("out of bound exception");
        }
        memcpy(&info, stream + pos, sizeof(T));
        pos += sizeof(T);
    }
    
    pair<const char*, size_t> Data() const {
        return make_pair(stream, GetSize());
    }
};


class Buffer {
    char *buffer;
    size_t size;
    size_t capacity;
    void Reserve(size_t newCapacity) {
        if (newCapacity > this->capacity) {
            char *nw = new char[newCapacity];
            if (buffer != nullptr) {
                memcpy(nw, buffer, size);
                delete[] buffer;
            }
            buffer = nw;
            this->capacity = newCapacity;
        }
    }
public:
    Buffer() {
        buffer = nullptr;
        size = 0;
        capacity = 0;
    }
    
    void Add(const char *data, size_t length) {
        if (size + length > capacity) {
            Reserve(max(size * 2, size + length));
        }
        memcpy(buffer + size, data, length);
        size += length;
    }
    
    Stream* GetStream() {
        if (size <= sizeOfSize)
            return nullptr;
        
        sizeType requiredSize = *(sizeType *)buffer;
        if (size < requiredSize)
            return nullptr;
        
        return new Stream(buffer, requiredSize);
    }
    
    void FreeStream(Stream *stream) {
        sizeType requiredSize = *(sizeType *)buffer;
        size -= requiredSize;
        memmove(buffer, buffer + requiredSize, size);
        delete stream;
    }
    
    void Dispose() {
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
            size = 0;
            capacity = 0;
        }
    }
    
    virtual ~Buffer() {
        Dispose();
    }

};


class Snake {
    vector<SDL_Point> positions;
    direction dir;
    direction new_dir;
    
public:
    Snake() {
        new_dir = dir = None;
        positions.push_back({20, 20});
    }
    void Progress() {
        for (int i=positions.size() - 1; i>0; i--)
            positions[i] = positions[i - 1];
        
        if (dir == None || abs(new_dir - dir) != 2 ) {
            dir = new_dir;
        }
        
        positions[0] = positions[0] + delta[dir];
        if (positions[0].x > 100 - 1)
            positions[0].x = 0; 
        else if (positions[0].y > 100 - 1)
            positions[0].y = 0;
        else if (positions[0].x < 0)
            positions[0].x = 100 - 1;
        else if (positions[0].y < 0)
            positions[0].y = 100 - 1;
        
//        mvprintw(0, 0, "%d %d", positions[0].y, positions[0].x);
    }
    void Turn(direction newDir) {
        new_dir = newDir;   
    }
    void Draw(SDL_Renderer* renderer) {
        for (auto &it: positions) {
            SDL_RenderDrawLine(renderer, (it.x)*8,     (it.y)*8,     (it.x + 1)*8, (it.y)*8    );
            SDL_RenderDrawLine(renderer, (it.x + 1)*8, (it.y)*8,     (it.x + 1)*8, (it.y + 1)*8);
            SDL_RenderDrawLine(renderer, (it.x + 1)*8, (it.y + 1)*8, (it.x)*8,     (it.y + 1)*8);
            SDL_RenderDrawLine(renderer, (it.x)*8,     (it.y + 1)*8, (it.x)*8,     (it.y)*8    );
        }
    }
    void Grow() {
        positions.push_back(positions.back());
    }
    
    vector<SDL_Point>& getPositions() {
        return positions;
    }
    
    void Serialize(Stream &stream) {
        stream.Push(positions.size());
        for (auto &it: positions) {
            stream.Push(it);
        }
        stream.Push(dir);
        stream.Push(new_dir);
    }
    void Deserialize(Stream &stream) {
        size_t n;
        stream.Pull(n);
        positions.clear();
        positions.resize(n);
        for (auto &it: positions) {
            stream.Pull(it);
        }
        stream.Pull(dir);
        stream.Pull(new_dir);
    }
};

class Berry{
    SDL_Point pos;
    
public:
    Berry(SDL_Point pos) {
        this->pos = pos;
    }
    SDL_Point getPosition() const {
        return pos;
    }
    void Draw(SDL_Renderer* renderer) {
        SDL_RenderDrawLine(renderer, (pos.x)*8 + 4, (pos.y)*8,     (pos.x + 1)*8, (pos.y)*8 + 4);
        SDL_RenderDrawLine(renderer, (pos.x + 1)*8, (pos.y)*8 + 4, (pos.x)*8 + 4, (pos.y + 1)*8);
        SDL_RenderDrawLine(renderer, (pos.x)*8 + 4, (pos.y + 1)*8, (pos.x)*8,     (pos.y)*8 + 4);
        SDL_RenderDrawLine(renderer, (pos.x)*8,     (pos.y)*8 + 4, (pos.x)*8 + 4, (pos.y)*8    );
    }
    void Serialize(Stream &stream) {
        stream.Push(pos);
    }
    void Deserialize(Stream &stream) {
        stream.Pull(pos);
    }
};

class Game {
    map<int, Snake> snakes;
    Berry b;
public:
    Game():b(Berry({10, 30})) {
    }
    void AddSnake(int id) {
        snakes.insert(make_pair(id, Snake()));
    }
    void RemoveSnake(int id) {
        snakes.erase(id);
    }
    void Progress() {
        for (auto &it: snakes) {
            it.second.Progress();
            if (it.second.getPositions()[0] == b.getPosition()) {
                it.second.Grow();
                b = Berry({rand() % 100, rand() % 100});
            }
        }
    }
    void Turn(int index, direction dir) {
        if (snakes.count(index) == 0) {
            fprintf(stderr, "CRITICAL WARNING!! SNAKE ISN'T IN THE MAP");
        }
        snakes.at(index).Turn(dir);
    }
    void Grow_tmp(int index) {
        if (snakes.count(index) == 0) {
            fprintf(stderr, "CRITICAL WARNING!! SNAKE ISN'T IN THE MAP");
        }
        snakes.at(index).Grow();
    }
    
    void Draw(SDL_Renderer* renderer) {
        b.Draw(renderer);
        for (auto &it: snakes) {
            it.second.Draw(renderer);
        }
    }
    
    void Serialize(Stream &stream) {
        stream.Clear();
        stream.Push(snakes.size());
        for (auto &it: snakes) {
            stream.Push(it.first);
            it.second.Serialize(stream);
        }
        b.Serialize(stream);
    }
    
    void Deserialize(Stream &stream) {
        stream.ResetRead();
        size_t n;
        stream.Pull(n);
        snakes.clear();
        for (int i=0; i<n; i++) {
            int index;
            stream.Pull(index);
            snakes[index] = Snake();
            snakes[index].Deserialize(stream);
        }
        b.Deserialize(stream);
    }
};

const int broad = 8745;
const int resp = 8744;
const int serverPort = 9874;

bool FindServer(struct sockaddr_in *serv_addr) {
    UdpSender broadcast(true, broad);
    UdpCatcher responds(resp);
    
    bool ServerIsFound = false;
    sockaddr_in tmp;
    
    broadcast.Send(NULL);
    for (int i=0; i<100; i++) {
        if (responds.TryRecv(&tmp, 1000)) {
            ServerIsFound = true;
            break;
        }
    }
    
    broadcast.Close();
    responds.Close();
    
    if (ServerIsFound) {
        memcpy(serv_addr, &tmp, sizeof(struct sockaddr_in));
        serv_addr->sin_port = htons(serverPort);
        return true;
    }
    else {
        memset(serv_addr, 0, sizeof(sockaddr_in));
        serv_addr->sin_family = AF_INET;
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr->sin_addr) != 1)
            throw runtime_error("inet_pton error");
        serv_addr->sin_port = htons(serverPort);
        return false;
    }
}

void InitServer(UdpCatcher **broadcast, UdpSender **response) {
    *broadcast = new UdpCatcher(broad);
    *response = new UdpSender(false, resp);
}

bool TryAnswerToClients(UdpCatcher *broadcast, UdpSender *response, struct sockaddr_in *from, int usecs) {
    sockaddr_in _from;
    
    if (broadcast->TryRecv(&_from, usecs)) {
        response->Send(&_from);
        memcpy(from, &_from, sizeof(struct sockaddr_in));
        return true;
    }
    
    return false;
}

static bool Exit = false;

Game TheGame;
Game aGame;

void HandleNewClient(int id) {
    TheGame.AddSnake(id);
    printf("Client with id %d connected\n", id);
}


map<char, direction> tturn = {
    {'w', North},
    {'d', East},
    {'s', South},
    {'a', West},
};
void HandleNewMessage(int id, const char *message, size_t length) {
    if (length == 1) {
        if (tturn.count(message[0]) == 1) {
            TheGame.Turn(id, tturn[message[0]]);
        } else if (message[0] == 'l') {
            TheGame.Grow_tmp(id);
        }
    }
    return;
}

void HandleOldClient(int id) {
    TheGame.RemoveSnake(id);
    printf("Client with id %d disconnected\n", id);
}

pthread_mutex_t clientMutex;

void* ServerThread(void *data) {
    //UDP
    UdpCatcher *broadcast;
    UdpSender *response;
    InitServer(&broadcast, &response);
    sockaddr_in client_addr;
    
    //TCP
    TcpServer server(serverPort, HandleNewClient, HandleNewMessage, HandleOldClient);
    
    int progress = 0;
    Stream stream;
//    TheGame.AddSnake(-1);
//    TheGame.Turn(-1, East);
    while (true) {
        pthread_mutex_lock(&clientMutex);
        if (Exit) {
            pthread_mutex_unlock(&clientMutex);
            break;
        }
        pthread_mutex_unlock(&clientMutex);
        
        if (TryAnswerToClients(broadcast, response, &client_addr, 1000))
            printf("Got client at %s:%hu\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        server.HandleNewEvents(1000);
        
        if (++progress % 20 == 0) {
            TheGame.Progress();
            TheGame.Serialize(stream);
            auto data = stream.Data();
            if (data.second == 0)
                throw runtime_error("why?");
            server.SendToAll(data.first, data.second);
            stream.Clear();
        }
    }
    broadcast->Close();
    response->Close();
    server.Close();
    
    pthread_exit(NULL);
}

char key;

//static int errr = 0;

Buffer clientBuffer;
//Dispose later

void HandleNewMessageClient(const char *message, size_t length) {
    clientBuffer.Add(message, length);
    
    Stream *stream = clientBuffer.GetStream();
    if (stream != nullptr) {
        pthread_mutex_lock(&clientMutex);
        aGame.Deserialize(*stream);
        pthread_mutex_unlock(&clientMutex);
        clientBuffer.FreeStream(stream);
    }
//        fprintf(stdout, "--- %lu\n", length);
//    } else {
//        errr++;
////        if (errr % 10 == 0)
//            fprintf(stderr, "%d %lu\n", errr, length);
//    }
    return;
}

// TODO: IMPLEMENT normal stream!!!

void* ClientThread(void *data) {
    struct sockaddr_in server_addr;
    memcpy(&server_addr, data, sizeof(sockaddr_in));
    
    TcpClient client(&server_addr, HandleNewMessageClient);
    while (true) {
        pthread_mutex_lock(&clientMutex);
        if (Exit) {
            pthread_mutex_unlock(&clientMutex);
            break;
        }
        pthread_mutex_unlock(&clientMutex);
        
        client.HandleNewEvents(1000);
        if (client.Closed()) {
            pthread_mutex_lock(&clientMutex);
            Exit = true;
            pthread_mutex_unlock(&clientMutex);
            break;
        }
        
        pthread_mutex_lock(&clientMutex);
        // if connected !!!!!!! TODOO DODODODOo
        if (key) {
            client.Send(&key, 1);
            key = 0;
        } 
        pthread_mutex_unlock(&clientMutex);
    }
    client.Close();
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
//    crcInit();
    
    sockaddr_in server_addr;
    pthread_t serverThread = -1, clientThread = -1;
    
    bool serverFound = FindServer(&server_addr);
    if (serverFound) {
        printf("Server found at: %s\n", inet_ntoa(server_addr.sin_addr));
    }
    else {
        printf("Server not Found. I'm the server now. Acquiring clients:\n");
        pthread_create(&serverThread, NULL, ServerThread, NULL);
    }
    usleep(100);
    pthread_mutex_init(&clientMutex, NULL);
    key = 0;
    pthread_create(&clientThread, NULL, ClientThread, &server_addr);
    
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;

        uint32_t old, nw;
        old = SDL_GetTicks();
        if (SDL_CreateWindowAndRenderer(800, 800, 0, &window, &renderer) == 0) {
            SDL_bool done = SDL_FALSE;

            while (!done) {
                SDL_Event event;

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);
                
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                pthread_mutex_lock(&clientMutex);
                aGame.Draw(renderer);
                pthread_mutex_unlock(&clientMutex);
                
                SDL_RenderPresent(renderer);
                
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        done = SDL_TRUE;
                    }
                    if (event.type == SDL_KEYDOWN) {
                        pthread_mutex_lock(&clientMutex);
                        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                            done = SDL_TRUE;
                        // EXIT =TRUE
                        
                        if (event.key.keysym.scancode == SDL_SCANCODE_W)
                            key = 'w';
                        else if (event.key.keysym.scancode == SDL_SCANCODE_D)
                            key = 'd';
                        else if (event.key.keysym.scancode == SDL_SCANCODE_S)
                            key = 's';
                        else if (event.key.keysym.scancode == SDL_SCANCODE_A)
                            key = 'a';
                        else if (event.key.keysym.scancode == SDL_SCANCODE_L)
                            key = 'l';
                        else 
                            key = 0;
                            
//                        if (event.key.keysym.scancode == SDL_SCANCODE_P)
//                            g.Grow_tmp(0);
                        pthread_mutex_unlock(&clientMutex);
                    }
                }
                
                nw = SDL_GetTicks();
                SDL_SetWindowTitle(window, (nw - old == 0)?"many":to_string(1000. /(nw - old)).c_str());
                old = nw;
                SDL_Delay(30);
            }
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
    SDL_Quit();
    
    pthread_mutex_lock(&clientMutex);
    Exit = true;
    pthread_mutex_unlock(&clientMutex);
    if (serverThread != -1)
        pthread_join(serverThread, NULL);
    if (clientThread != -1)
        pthread_join(clientThread, NULL);
    return 0;
}

