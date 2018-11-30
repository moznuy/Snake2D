/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Memory.h
 * Author: lamar
 *
 * Created on November 20, 2017, 4:23 PM
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <cstdlib>
#include <stdexcept>
#include <cstring>

typedef unsigned int sizeType;
const int sizeOfSize = sizeof(sizeType);

struct StreamData {
    const char *data;
    size_t length;
};

class Stream {
private:
    char *stream;
    size_t capacity;
    size_t pos;
    bool own;
    
    void SetSize(sizeType size);
    void Dispose(bool completly);
    void Reserve(size_t newCapacity);
public:
    Stream();
    Stream(char *data, size_t length);
    ~Stream();
    void Clear();
    
    template<typename T>
    void Push(const T &info);
    template<typename T>
    void Pull(T &info);
    
    sizeType GetSize() const;
    void ResetRead();
    StreamData Get() const;
};

class Buffer {
private:
    char *buffer;
    size_t size;
    size_t capacity;
    void Reserve(size_t newCapacity);
public:
    Buffer();
    void Add(const char *data, size_t length);
    Stream* GetStream();
    void FreeStream(Stream *stream);
    void Dispose();
    ~Buffer();
};

template<typename T>
void Stream::Push(const T &info) {
    if (!own)
        throw std::runtime_error("Stream is not mine");

    auto size = GetSize();
    if (size + sizeof(T) > capacity) {
        Reserve(max(sizeType(size * 2), sizeType(size + sizeof(T))));
    }
    memcpy(stream + size, &info, sizeof(T));
    size += sizeof(T);
    SetSize(size);
}

template<typename T>
void Stream::Pull(T &info) {
    if (sizeof(T) + pos > GetSize()) {
        throw std::runtime_error("out of bound exception");
    }
    memcpy(&info, stream + pos, sizeof(T));
    pos += sizeof(T);
}

#endif /* MEMORY_H */

