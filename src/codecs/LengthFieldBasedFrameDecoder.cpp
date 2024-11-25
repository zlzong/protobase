#include "LengthFieldBasedFrameDecoder.h"
#include "base/Logger.h"

#include <iostream>

LengthFieldBasedFrameDecoder::LengthFieldBasedFrameDecoder(size_t maxFrameLength, size_t lengthFieldOffset,
                                                           size_t lengthFieldLength, size_t lengthAdjustment,
                                                           size_t initialBytesToStrip)
    : m_lengthFieldOffset(lengthFieldOffset)
      , m_lengthFieldLength(lengthFieldLength)
      , m_lengthAdjustment(lengthAdjustment)
      , m_initialBytesToStrip(initialBytesToStrip)
      , m_maxFrameLength(maxFrameLength) {
    if (m_lengthFieldLength != 1 && m_lengthFieldLength != 2 && m_lengthFieldLength != 4 && m_lengthFieldLength != 8) {
        LOG_ERROR("Invalid length field: {}", m_lengthFieldLength);
    }
}

Buffer LengthFieldBasedFrameDecoder::decode(Buffer *buffer) {
    LOG_INFO("decode called");
    Buffer buf{};
    size_t readableBytes = buffer->readableBytes();
    if (readableBytes < m_lengthFieldLength + m_lengthFieldOffset) {
        return buf;
    }

    size_t payLoadLength = 0;
    switch (m_lengthFieldLength) {
        case 1:
            payLoadLength = *buffer->peek(m_lengthFieldOffset) & 0xff;
            break;
        case 2:
            payLoadLength = *reinterpret_cast<uint16_t *>(buffer->peek(m_lengthFieldOffset));
            break;
        case 4:
            payLoadLength = *reinterpret_cast<uint32_t *>(buffer->peek(m_lengthFieldOffset));
            break;
        case 8:
            payLoadLength = *reinterpret_cast<uint64_t *>(buffer->peek(m_lengthFieldOffset));
            break;
        default:
            LOG_ERROR("Invalid length field: {}", m_lengthFieldLength);
    }

    size_t frameLength = m_lengthFieldOffset + m_lengthFieldLength + payLoadLength + m_lengthAdjustment;
    if (frameLength > m_maxFrameLength) {
        LOG_ERROR("frame length exceeds maximum length: {}", frameLength);
        throw std::overflow_error("frame length exceeds maximum length");
    }

    if (readableBytes < frameLength) {
        return buf;
    }

    buf.append(buffer->peek(), frameLength);
    buffer->skip(frameLength);
    return buf;
}
