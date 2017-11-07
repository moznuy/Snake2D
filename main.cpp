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

#include <cstdlib>
#include <bits/stdc++.h>
#include <SDL2/SDL.h>

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

class Snake {
    vector<SDL_Point> positions;
    direction dir;
    
public:
    Snake() {
        dir = None;
        positions.push_back({20, 20});
    }
    void Progress() {
        for (int i=positions.size() - 1; i>0; i--)
            positions[i] = positions[i - 1];
        
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
        if (newDir == None || abs(newDir - dir) == 2 && dir != None)
            return;
        
        dir = newDir;   
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
};

class Game {
    vector<Snake> snakes;
    Berry b;
public:
    Game():b(Berry({10, 30})) {
    }
    void AddSnake() {
        snakes.push_back(Snake());
    }
    void Progress() {
        for (auto &it: snakes) {
            it.Progress();
            if (it.getPositions()[0] == b.getPosition()) {
                it.Grow();
                b = Berry({rand() % 100 + 1, rand() % 100 + 2});
            }
        }
    }
    void Turn(int index, direction dir) {
        if (index >= 0 && index < snakes.size())
            snakes[index].Turn(dir);
    }
    void Grow_tmp(int index) {
        if (index >= 0 && index < snakes.size())
            snakes[index].Grow();
    }
    
    void Draw(SDL_Renderer* renderer) {
        b.Draw(renderer);
        for (auto &it: snakes) {
            it.Draw(renderer);
        }
    }
    
};

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
//        SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;

        uint32_t old, nw;
        old = SDL_GetTicks();
        if (SDL_CreateWindowAndRenderer(800, 800, 0, &window, &renderer) == 0) {
            SDL_bool done = SDL_FALSE;
            
            Game g;
            g.AddSnake();

            while (!done) {
                SDL_Event event;

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);

//                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
//                SDL_RenderDrawLines(renderer, points, POINTS_COUNT);
//                for (int i=0; i<500; i++) {
//                    for (int j=0; j<300; j++) {
////                        SDL_SetRenderDrawColor(renderer, (i + j*2)%255 , (2*i - j)%255, (3*i + 5*j/3)%255 , SDL_ALPHA_OPAQUE);
//                        SDL_RenderDrawPoint(renderer, i, j);
//                    }
//                }
                
                g.Progress();
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                g.Draw(renderer);
                
                SDL_RenderPresent(renderer);
                
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        done = SDL_TRUE;
                    }
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.scancode == SDL_SCANCODE_W)
                            g.Turn(0, North);
                        if (event.key.keysym.scancode == SDL_SCANCODE_D)
                            g.Turn(0, East);
                        if (event.key.keysym.scancode == SDL_SCANCODE_S)
                            g.Turn(0, South);
                        if (event.key.keysym.scancode == SDL_SCANCODE_A)
                            g.Turn(0, West);
                        if (event.key.keysym.scancode == SDL_SCANCODE_P)
                            g.Grow_tmp(0);
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
    return 0;
}
