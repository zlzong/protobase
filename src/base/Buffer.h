#pragma once

#include "Types.h"

#include <string>
#include <cstring>
#include <cstdint>
#include <atomic>

class Buffer {
public:
    static const size_t kPrependSize = 8;
    static const size_t kInitialSize = 1450;

    explicit Buffer(size_t initialSize = kInitialSize)
            : m_readIndex(kPrependSize),
              m_writeIndex(kPrependSize),
              m_capacity(kPrependSize + initialSize),
              m_ownMemory(true) {
        m_buffer = new char[m_capacity];
        m_refCount = new std::atomic<int>(1);
    }

    Buffer(char* buffer, size_t capacity, size_t readIndex, size_t writeIndex, std::atomic<int>* refCount)
            :m_buffer(buffer),
            m_readIndex(readIndex),
            m_writeIndex(writeIndex),
            m_capacity(capacity),
            m_refCount(refCount),
            m_ownMemory(false) {
        m_refCount->fetch_add(1, std::memory_order_relaxed);
    }

    ~Buffer() {
        if (m_refCount) {
            if (m_refCount->fetch_sub(1, std::memory_order_release) == 1) {
                std::atomic_thread_fence(std::memory_order_acquire);
                delete[] m_buffer;
                delete m_refCount;
            }

            m_buffer = nullptr;
            m_refCount = nullptr;
            m_capacity = 0;
        }
    }

    Buffer(const Buffer& other)
            :m_buffer(other.m_buffer),
            m_readIndex(other.m_readIndex),
            m_writeIndex(other.m_writeIndex),
            m_capacity(other.m_capacity),
            m_refCount(other.m_refCount),
            m_ownMemory(false){
        m_refCount->fetch_add(1, std::memory_order_relaxed);
    }

    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            if (m_refCount) {
                if (m_refCount->fetch_sub(1, std::memory_order_release) == 1) {
                    std::atomic_thread_fence(std::memory_order_acquire);
                    delete[] m_buffer;
                    delete m_refCount;
                }
            }

            m_buffer = other.m_buffer;
            m_capacity = other.m_capacity;
            m_readIndex = other.m_readIndex;
            m_writeIndex = other.m_writeIndex;
            m_refCount = other.m_refCount;
            m_ownMemory = false;

            m_refCount->fetch_add(1, std::memory_order_relaxed);
        }

        return *this;
    }

    void swap(Buffer& other) noexcept {
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_readIndex, other.m_readIndex);
        std::swap(m_writeIndex, other.m_writeIndex);
    }

    Buffer(Buffer&& other) noexcept : m_buffer(other.m_buffer), m_readIndex(other.m_readIndex), m_writeIndex(other.m_writeIndex), m_capacity(other.m_capacity), m_refCount(other.m_refCount), m_ownMemory(other.m_ownMemory) {
        other.m_buffer = nullptr;
        other.m_capacity = 0;
        other.m_readIndex = kPrependSize;
        other.m_writeIndex = kPrependSize;
        other.m_refCount = nullptr;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            if (m_refCount) {
                if (m_refCount->fetch_sub(1, std::memory_order_release) == 1) {
                    std::atomic_thread_fence(std::memory_order_acquire);
                    delete[] m_buffer;
                    delete m_refCount;
                }
            }

            m_buffer = other.m_buffer;
            m_capacity = other.m_capacity;
            m_readIndex = other.m_readIndex;
            m_writeIndex = other.m_writeIndex;
            m_refCount = other.m_refCount;
            m_ownMemory = other.m_ownMemory;

            other.m_buffer = nullptr;
            other.m_refCount = nullptr;
            other.m_capacity = 0;
            other.m_readIndex = kPrependSize;
            other.m_writeIndex = kPrependSize;
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

    void skip(size_t len);

    void writeN(size_t len);

    void reset();

    void readAll();

    std::string readAllAsString();

    std::string readAsString(size_t len);

    std::string readAllAsHexString();

    std::string readAsHexString(size_t len);

    void ensureWriteableBytes(size_t len);

    void makeOwnBuffer(size_t additionalSpace);

    void append(const char *data, size_t len);

    void append(const unsigned char *data, size_t len);

    void append(const void *data, size_t len);

    BufferPtr slice(size_t offset, size_t length);

    BufferPtr slicePacket(size_t length);

    char *beginWrite();

    const char *beginWrite() const;

    ssize_t readFd(int fd);

    char readChar();

    uint8_t readU8();

    uint16_t readU16LE();

    uint16_t readU16BE();

    uint32_t readU32LE();

    uint32_t readU32BE();

    uint8_t peekU8(int offset) const;

    uint16_t peekU16LE(int offset) const;

    uint16_t peekU16BE(int offset) const;

    uint32_t peekU32LE(int offset) const;

    uint32_t peekU32BE(int offset) const;

    BufferPtr readBuffer(int len);

    size_t writeFd(int fd);

    size_t capacity();

    void appendU16BE(uint16_t u16);

    void appendU16LE(uint16_t u16);

    void appendU32BE(uint32_t u32);

    void appendU32LE(uint32_t u32);

    void appendFloat(float num);

    void appendChar(char c);

    void appendU8(uint8_t us);

private:

    char *begin();

    const char *begin() const;

    void makeSpace(size_t len);

    char *m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;
    size_t m_capacity;
    std::atomic<int>* m_refCount;
    bool m_ownMemory;
};