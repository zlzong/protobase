#pragma once

#include "Types.h"
#include "Logger.h"

#include <string>
#include <cstdint>

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
            : m_readIndex(kCheapPrepend),
              m_writeIndex(kCheapPrepend),
              m_capacity(kCheapPrepend + initialSize) {
        m_buffer = new char[m_capacity];
    }

    ~Buffer() {
        delete[] m_buffer;
        m_buffer = nullptr;
        m_capacity = 0;
    }

    Buffer(const Buffer& other) {
        m_buffer = new char[other.m_capacity];
        m_readIndex = other.m_readIndex;
        m_writeIndex = other.m_writeIndex;
        m_capacity = other.m_capacity;
        memcpy(m_buffer, other.m_buffer, other.m_capacity);
    }

    Buffer& operator=(const Buffer& other) {
        Buffer temp(other);
        swap(temp);
        return *this;
    }

    void swap(Buffer& other) noexcept {
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_readIndex, other.m_readIndex);
        std::swap(m_writeIndex, other.m_writeIndex);
    }

    Buffer(Buffer&& other) noexcept : m_buffer(other.m_buffer), m_capacity(other.m_capacity), m_readIndex(other.m_readIndex), m_writeIndex(other.m_writeIndex) {
        other.m_buffer = nullptr;
        other.m_capacity = 0;
        other.m_readIndex = kCheapPrepend;
        other.m_writeIndex = kCheapPrepend;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            swap(other);
        }

        return *this;
    }

    size_t readableBytes() const;

    size_t writableBytes() const;

    size_t prependableBytes() const;

    const char *peek() const;

    char *peek();

    const char *peek(size_t offset) const;

    char *peek(size_t offset);

    void retrieve(size_t len);

    void writeN(size_t len);

    void reset();

    void retrieveAll();

    std::string retrieveAllString();

    std::string retrieveAllHexString();

    std::string retrieveAsString(size_t len);

    std::string retrieveAsHexString(size_t len);

    void ensureWriteableBytes(size_t len);

    void append(const char *data, size_t len);

    void append(const unsigned char *data, size_t len);

    void append(const void *data, size_t len);

    char *beginWrite();

    const char *beginWrite() const;

    size_t readFd(int fd);

    char readChar();

    unsigned char readUChar();

    BufferPtr readBuffer(int len);

    uint16_t readU16LE();

    uint16_t peekU16LE(int offset) const;

    uint16_t readU16BE();

    uint32_t readU32LE();

    uint32_t readU32BE();

    size_t writeFd(int fd);

    size_t capacity();

    void appendU16BE(uint16_t u16);

    void appendU16LE(uint16_t u16);

    void appendU32BE(uint32_t u32);

    void appendU32LE(uint32_t u32);

    void appendFloat(float num);

    void appendChar(char c);

    void appendUChar(unsigned char us);

private:

    char *begin();

    const char *begin() const;

    void makeSpace(size_t len);

    char *m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;
    size_t m_capacity;
};