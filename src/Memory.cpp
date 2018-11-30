/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <algorithm>
#include "Memory.h"

using namespace std;
   
void Stream::SetSize(sizeType size) {
    if (!own)
        throw runtime_error("Stream is not mine");

    *(sizeType *)stream = size;
}

void Stream::Dispose(bool completly) {
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

void Stream::Reserve(size_t newCapacity) {
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


Stream::Stream() {
    stream = new char[sizeOfSize];
    capacity = sizeOfSize;

    own = true;
    Clear();
}

Stream::Stream(char *data, size_t length) {
    stream = data;
    capacity = length;
    pos = sizeOfSize;

    own = false;
}

sizeType Stream::GetSize() const {
    return *(sizeType *)stream;
}

void Stream::Clear() {
    if (!own)
        throw runtime_error("Stream is not mine");

    SetSize(sizeOfSize);
    ResetRead();
}



Stream::~Stream() {
    Dispose(true);
}

void Stream::ResetRead() {
    pos = sizeOfSize;
}

StreamData Stream::Get() const {
    return {stream, GetSize()};
}

void Buffer::Reserve(size_t newCapacity) {
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

Buffer::Buffer() {
    buffer = nullptr;
    size = 0;
    capacity = 0;
}

void Buffer::Add(const char *data, size_t length) {
    if (size + length > capacity) {
        Reserve(max(size * 2, size + length));
    }
    memcpy(buffer + size, data, length);
    size += length;
}

Stream* Buffer::GetStream() {
    if (size <= sizeOfSize)
        return nullptr;

    sizeType requiredSize = *(sizeType *)buffer;
    if (size < requiredSize)
        return nullptr;

    return new Stream(buffer, requiredSize);
}

void Buffer::FreeStream(Stream *stream) {
    sizeType requiredSize = *(sizeType *)buffer;
    size -= requiredSize;
    memmove(buffer, buffer + requiredSize, size);
    delete stream;
}

void Buffer::Dispose() {
    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
        size = 0;
        capacity = 0;
    }
}

Buffer::~Buffer() {
    Dispose();
}
