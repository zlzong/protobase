#pragma once

#include "Types.h"
#include "Logger.h"

#include <vector>
#include <string>
#include <cstdint>

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
            : m_buffer(kCheapPrepend + initialSize),
              m_readIndex(kCheapPrepend),
              m_writeIndex(kCheapPrepend),
              m_capacity(kCheapPrepend + initialSize) {

    }

    ~Buffer() = default;

    size_t readableBytes() const;

    size_t writableBytes() const;

    size_t prependableBytes() const;

    const char *peek() const;

    char *peek();

    void retrieve(size_t len);

    void writeN(size_t len);

    void reset();

    void retrieveAll();

    std::string retrieveAllString();

    std::string retrieveAsString(size_t len);

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

    std::vector<char> m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;
    size_t m_capacity;
};