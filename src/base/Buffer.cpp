#include "Buffer.h"
#include "Logger.h"

#include <string>
#include <unistd.h>
#include <sys/uio.h>
#include <sstream>
#include <iomanip>

size_t Buffer::readableBytes() const {
    return m_writeIndex - m_readIndex;
}

size_t Buffer::writableBytes() const {
    return m_capacity - m_writeIndex;
}


size_t Buffer::prependableBytes() const {
    return m_readIndex;
}

const char *Buffer::peek() const {
    return begin() + m_readIndex;
}

char *Buffer::peek() {
    return begin() + m_readIndex;
}

const char *Buffer::peek(size_t offset) const {
    return begin() + m_readIndex + offset;
}

char *Buffer::peek(size_t offset) {
    return begin() + m_readIndex + offset;
}

void Buffer::skip(size_t len) {
    if (len < readableBytes()) {
        m_readIndex += len;
    } else {
        readAll();
    }
}

void Buffer::writeN(size_t len) {
    m_writeIndex += len;
}

void Buffer::reset() {
    m_readIndex = kPrependSize;
    m_writeIndex = kPrependSize;
}

void Buffer::readAll() {
    m_readIndex = m_writeIndex = kPrependSize;
}

std::string Buffer::readAllAsString() {
    return readAsString(readableBytes());
}

std::string Buffer::readAsString(size_t len) {
    std::string result(peek(), len);
    skip(len);
    return result;
}

std::string Buffer::readAllAsHexString() {
    return readAsHexString(readableBytes());
}

std::string Buffer::readAsHexString(size_t len) {
    std::stringstream hex;
    hex << std::hex << std::uppercase << std::setfill('0');
    for (int i = 0; i < len; ++i) {
        hex << std::setw(2) << static_cast<int>(*reinterpret_cast<unsigned char *>(peek(i)));
    }

    skip(len);
    return hex.str();
}

void Buffer::ensureWriteableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
}

void Buffer::append(const char *data, size_t len) {
    ensureWriteableBytes(len);
    memcpy(beginWrite(),data, len);
    m_writeIndex += len;
}

void Buffer::append(const unsigned char *data, size_t len) {
    if (len <= 0) {
        LOG_ERROR("data is null, len: {}", len);
        return;
    }
    ensureWriteableBytes(len);
    memcpy(beginWrite(),data, len);
    m_writeIndex += len;
}

void Buffer::append(const void *data, size_t len) {
    append(static_cast<const char *>(data), len);
}

void Buffer::appendU16LE(uint16_t u16) {
    uint16_t u16BE = htole16(u16);
    append(&u16BE, sizeof(u16BE));
}

void Buffer::appendU16BE(uint16_t u16) {
    uint16_t u16LE = htobe16(u16);
    append(&u16LE, sizeof(u16LE));
}

void Buffer::appendU32LE(uint32_t u32) {
    uint32_t u32BE = htole32(u32);
    append(&u32BE, sizeof(u32BE));
}

void Buffer::appendU32BE(uint32_t u32) {
    uint32_t u32LE = htobe32(u32);
    append(&u32LE, sizeof(u32LE));
}

void Buffer::appendFloat(float num) {
    append(&num, sizeof(num));
}

void Buffer::appendChar(char c) {
    append(&c, sizeof(c));
}

void Buffer::appendU8(uint8_t us) {
    append(&us, sizeof(us));
}

char *Buffer::beginWrite() {
    return begin() + m_writeIndex;
}

const char *Buffer::beginWrite() const {
    return begin() + m_writeIndex;
}

size_t Buffer::readFd(int fd) {
    char extraBuf[65536];

    struct iovec vec[2];

    const size_t writeable = writableBytes();
    vec[0].iov_base = begin() + m_writeIndex;
    vec[0].iov_len = writeable;

    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof extraBuf;


    const int iovcnt = (writeable < sizeof extraBuf) ? 2 : 1;
    const ssize_t nRead = readv(fd, vec, iovcnt);
    if (nRead < 0) {
        LOG_ERROR("read from fd: {} error, errno:{}", fd, errno);
    } else if (nRead <= writeable) {
        m_writeIndex += nRead;
    } else {
        m_writeIndex = m_capacity;
        append(extraBuf, nRead - writeable);
    }

    return nRead;
}

char Buffer::readChar() {
    char c = *peek();
    skip(sizeof(c));
    return c;
}

unsigned char Buffer::readU8() {
    const uint8_t num = peekU8(0);
    skip(sizeof(num));
    return num;
}

uint16_t Buffer::readU16LE() {
    const uint16_t num = peekU16LE(0);
    skip(sizeof(num));
    return num;
}

uint16_t Buffer::readU16BE() {
    return 0;
}

uint32_t Buffer::readU32LE() {
    const uint32_t num = peekU32LE(0);
    skip(sizeof(num));
    return num;
}

uint32_t Buffer::readU32BE() {
    return 0;
}

uint8_t Buffer::peekU8(int offset) const {
    const char *start = peek() + offset;
    const auto num = static_cast<const uint8_t>(*start);
    return num;
}

uint16_t Buffer::peekU16LE(int offset) const {
    const char *start = peek() + offset;
    const uint16_t num = *reinterpret_cast<const uint16_t *>(start);
    return num;
}

uint32_t Buffer::peekU32LE(int offset) const {
    const char *start = peek() + offset;
    const uint32_t num = *reinterpret_cast<const uint32_t *>(start);
    return num;
}

BufferPtr Buffer::readBuffer(int len) {
    BufferPtr buffer = std::make_shared<Buffer>(len);
    buffer->append(peek(), len);
    skip(len);
    return buffer;
}

size_t Buffer::writeFd(int fd) {
    ssize_t nWrite = write(fd, peek(), readableBytes());
    if (nWrite < 0) {
        LOG_ERROR("write to fd error:{}", errno);
    }

    return nWrite;
}

size_t Buffer::capacity() {
    return m_capacity;
}

char *Buffer::begin() {
    return m_buffer;
}

const char *Buffer::begin() const {
    return m_buffer;
}

void Buffer::makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kPrependSize) {
        const size_t resizedCapacity = (m_capacity << 1) + len;
        const size_t readable = readableBytes();
        const auto d = new char[resizedCapacity];
        memcpy(d + kPrependSize, begin() + m_readIndex, readable);
        m_writeIndex = readable + kPrependSize;
        m_readIndex = kPrependSize;
        m_capacity = resizedCapacity;
        delete[] m_buffer;
        m_buffer = d;
    } else {
        size_t readable = readableBytes();
        memmove(begin() + kPrependSize, begin() + m_readIndex, readable);
        m_readIndex = kPrependSize;
        m_writeIndex = m_readIndex + readable;
    }
}