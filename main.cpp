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
// scp ./dist/Release/GNU-Linux/snake2d sberko@10.70.80.96:/home/sberko/Tmp/snake2d

#include <cstdlib>
#include <bits/stdc++.h>
#include <SDL2/SDL.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "BroadCast.h"
#include "Memory.h"

using namespace std;

enum direction {
    None,
    North,
    East,
    South,
    West,
};

struct Point {
    int x, y;
    static const int modulus = 101;
    Point() {
        x = y = 0;
    }
    Point(int x, int y) {
        this->x = x;
        this->y = y;
    }
    Point(Stream &s) {
        uint16_t pos;
        s.Pull(pos);
        
        x = pos / modulus;
        y = pos % modulus;
    }
    
    Point operator+(const Point &arg) {
        return {x + arg.x, y + arg.y};
    }
    bool operator==(const Point &arg) {
        return x == arg.x && y == arg.y;
    }
    
    static map<direction, Point> delta;
    
    void Serialize(Stream &s) {
        s.Push(uint16_t(x * modulus + y));
    }
};

map<direction, Point> Point::delta = {
    {None,  {0,  0}}, 
    {North, {0, -1}}, 
    {East,  {1,  0}},
    {South, {0,  1}},
    {West,  {-1, 0}},
};

class Snake {
    vector<Point> positions;
    direction dir;
    direction new_dir;
    
public:
    Snake() {
        new_dir = dir = None;
        positions.push_back({20, 20});
    }
    Snake(Stream &stream) {
        size_t n;
        stream.Pull(n);
        positions.resize(n);
        for (auto &it: positions) {
            it = Point(stream);
        }
        stream.Pull(dir);
        stream.Pull(new_dir);
    }
    void Progress() {
        for (int i=positions.size() - 1; i>0; i--) {
            positions[i] = positions[i - 1];
        }
        
        if (dir == None || abs(new_dir - dir) != 2 ) {
            dir = new_dir;
        }
        
        positions[0] = positions[0] + Point::delta[dir];
        if (positions[0].x > 100 - 1) {
            positions[0].x = 0; 
        }
        else if (positions[0].y > 100 - 1) {
            positions[0].y = 0;
        }
        else if (positions[0].x < 0) {
            positions[0].x = 100 - 1;
        }
        else if (positions[0].y < 0) {
            positions[0].y = 100 - 1;
        }
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
    
    vector<Point>& getPositions() {
        return positions;
    }
    
    void Serialize(Stream &stream) {
        stream.Push(positions.size());
        for (auto &it: positions) {
            it.Serialize(stream);
        }
        stream.Push(dir);
        stream.Push(new_dir);
    }
};

class Berry{
    Point pos;
    
public:
    Berry() {
    }
    Berry(Point pos) {
        this->pos = pos;
    }
    Berry(Stream &stream) {
        pos = Point(stream);
    }
    Point getPosition() const {
        return pos;
    }
    void Draw(SDL_Renderer* renderer) {
        SDL_RenderDrawLine(renderer, (pos.x)*8 + 4, (pos.y)*8,     (pos.x + 1)*8, (pos.y)*8 + 4);
        SDL_RenderDrawLine(renderer, (pos.x + 1)*8, (pos.y)*8 + 4, (pos.x)*8 + 4, (pos.y + 1)*8);
        SDL_RenderDrawLine(renderer, (pos.x)*8 + 4, (pos.y + 1)*8, (pos.x)*8,     (pos.y)*8 + 4);
        SDL_RenderDrawLine(renderer, (pos.x)*8,     (pos.y)*8 + 4, (pos.x)*8 + 4, (pos.y)*8    );
    }
    void Serialize(Stream &stream) {
        pos.Serialize(stream);
    }
};

class Game {
    map<int, Snake> snakes;
    Berry b;
public:
    Game():b(Berry({rand()%100, rand()%100})) {
    }
    
    Game(Stream &stream) {
        stream.ResetRead();
        size_t n;
        stream.Pull(n);
        for (size_t i=0; i<n; i++) {
            int index;
            stream.Pull(index);
            snakes[index] = Snake(stream);
        }
        b = Berry(stream);
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

mutex clientMutex;

void ServerThread() {
//    chrono::system_clock cl;
//    auto tp = cl.now();
//    int k = 0;
    
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
        clientMutex.lock();
        if (Exit) {
            clientMutex.unlock();
            break;
        }
        clientMutex.unlock();
        
        if (TryAnswerToClients(broadcast, response, &client_addr, 1000))
            printf("Got client at %s:%hu\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        /*k += */server.HandleNewEvents(1000);
        
//        auto tmp = cl.now();
//        if ((tmp - tp) >= chrono::seconds(1)) {
//            tp = tmp;
//            printf("%d\n", k);
//            k = 0;
//        }
        
        if (++progress % 20 == 0) {
            TheGame.Progress();
            TheGame.Serialize(stream);
            auto data = stream.Get();
            if (data.length == 0)
                throw runtime_error("why?");
            server.SendToAll(data.data, data.length);
            stream.Clear();
        }
    }
    broadcast->Close();
    response->Close();
    server.Close();
}

char key;


Buffer clientBuffer;
//Dispose later

void HandleNewMessageClient(const char *message, size_t length) {
    clientBuffer.Add(message, length);
    
    Stream *stream = clientBuffer.GetStream();
    if (stream != nullptr) {
        clientMutex.lock();
        aGame = Game(*stream);
        clientMutex.unlock();
        clientBuffer.FreeStream(stream);
    }
    return;
}

// TODO: IMPLEMENT normal stream!!!

void ClientThread(sockaddr_in *data) {
//    chrono::system_clock cl;
//    auto tp = cl.now();
//    int k = 0;
    
    struct sockaddr_in server_addr;
    memcpy(&server_addr, data, sizeof(sockaddr_in));
    
    TcpClient client(&server_addr, HandleNewMessageClient);
    while (true) {
        clientMutex.lock();
        if (Exit) {
            clientMutex.unlock();
            break;
        }
        clientMutex.unlock();
        
        if (client.HandleNewEvents(1000));
//            k++;
        
//        auto tmp = cl.now();
//        if ((tmp - tp) >= chrono::seconds(1)) {
//            tp = tmp;
//            printf("%d\n", k);
//            k = 0;
//        }
        
        if (client.Closed()) {
            clientMutex.lock();
            Exit = true;
            clientMutex.unlock();
            break;
        }
        
        clientMutex.lock();
        // if connected !!!!!!! TODOO DODODODOo
        if (key) {
            client.Send(&key, 1);
            key = 0;
        } 
        clientMutex.unlock();
    }
    client.Close();
}

int main(int argc, char *argv[]) {
//    crcInit();
    
    sockaddr_in server_addr;
    thread *serverThread = nullptr, *clientThread = nullptr;
    
    bool serverFound = FindServer(&server_addr);
    if (serverFound) {
        printf("Server found at: %s\n", inet_ntoa(server_addr.sin_addr));
    }
    else {
        printf("Server not Found. I'm the server now. Acquiring clients:\n");
        serverThread = new thread(ServerThread);
    }
    usleep(100);
    key = 0;
    clientThread = new thread(ClientThread, &server_addr);
    
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
                clientMutex.lock();
                aGame.Draw(renderer);
                clientMutex.unlock();
                
                SDL_RenderPresent(renderer);
                
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        done = SDL_TRUE;
                    }
                    if (event.type == SDL_KEYDOWN) {
                        clientMutex.lock();
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
                        clientMutex.unlock();
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
    
    clientMutex.lock();
    Exit = true;
    clientMutex.unlock();
    if (clientThread != nullptr)
        clientThread->join();
    if (serverThread != nullptr)
        serverThread->join();
    clientBuffer.Dispose();
    return 0;
}

