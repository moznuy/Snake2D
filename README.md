[![Build Status](https://travis-ci.org/moznuy/Snake2D.svg?branch=master)](https://travis-ci.org/moznuy/Snake2D)

# Download
## Windows

[Releases](../../releases)

# Compilation
## Linux

### Requiments:

- SDL2 dev (libsdl2-dev or similar)
- gcc
- make
- cmake

### Steps:

- `mkdir release && cd release`
- `cmake -DCMAKE_BUILD_TYPE=Release ..`
- `make`
- `./Snake2D`

## Windows

### Requiments:

- http://mingw-w64.org/doku.php/download/mingw-builds
- https://www.libsdl.org/download-2.0.php Mingw version
- TODO: explain setup

### Steps:

- `mkdir release && cd release`
- `cmake -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - MinGW Makefiles" ..`
- `mingw32-make`
- `Snake2D`
- TODO: Install or dll dependency explanation 
