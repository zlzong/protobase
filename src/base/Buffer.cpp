#include "Buffer.h"
#include "Logger.h"

#include <string>
#include <unistd.h>
#include <sys/uio.h>

size_t Buffer::readableBytes() const {
    return m_writeIndex - m_readIndex;
}

size_t Buffer::writableBytes() const {
    return m_buffer.size() - m_writeIndex;
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

void Buffer::retrieve(size_t len) {
    if (len < readableBytes()) {
        m_readIndex += len;
    } else {
        retrieveAll();
    }
}

void Buffer::writeN(size_t len) {
    m_writeIndex += len;
}

void Buffer::reset() {
    m_readIndex = kCheapPrepend;
    m_writeIndex = kCheapPrepend;
}

void Buffer::retrieveAll() {
    m_readIndex = m_writeIndex = kCheapPrepend;
}

std::string Buffer::retrieveAllString() {
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len) {
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

void Buffer::ensureWriteableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
}

void Buffer::append(const char *data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
    m_writeIndex += len;
}

void Buffer::append(const unsigned char *data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
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

void Buffer::appendUChar(unsigned char us) {
    append(&us, sizeof(us));
}

char *Buffer::beginWrite() {
    return begin() + m_writeIndex;
}

const char *Buffer::beginWrite() const {
    return begin() + m_writeIndex;
}

size_t Buffer::readFd(int fd) {
    char extraBuf[65536] = {0};

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
        m_writeIndex = m_buffer.size();
        append(extraBuf, nRead - writeable);
    }

    return nRead;
}

char Buffer::readChar() {
    char c = *peek();
    retrieve(sizeof(c));
    return c;
}

unsigned char Buffer::readUChar() {
    char *c = peek();
    unsigned char *uc = reinterpret_cast<unsigned char *>(c);
    retrieve(sizeof(char));
    return *uc;
}

BufferPtr Buffer::readBuffer(int len) {
    BufferPtr buffer = std::make_shared<Buffer>(len);
    buffer->append(peek(), len);
    retrieve(len);
    return buffer;
}

uint16_t Buffer::readU16LE() {
    const char *start = peek();
    uint16_t num = *(uint16_t *) start;
    retrieve(sizeof(num));
    return num;
}

uint16_t Buffer::peekU16LE(int offset) const {
    const char *start = peek() + offset;
    uint16_t num = *(uint16_t *) start;
    return num;
}

uint16_t Buffer::readU16BE() {
    return 0;
}

uint32_t Buffer::readU32LE() {
    const char *start = peek();
    uint32_t num = *(uint32_t *) start;
    retrieve(sizeof(num));
    return num;
}

uint32_t Buffer::readU32BE() {
    return 0;
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
    return &*m_buffer.begin();
}

const char *Buffer::begin() const {
    return &*m_buffer.begin();
}

void Buffer::makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
        m_buffer.resize(m_writeIndex + len);
    } else {
        size_t readable = readableBytes();
        std::copy(begin() + m_readIndex, begin() + m_writeIndex, begin() + kCheapPrepend);
        m_readIndex = kCheapPrepend;
        m_writeIndex = m_readIndex + readable;
    }
}