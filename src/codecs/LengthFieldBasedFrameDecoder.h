#pragma once

#include "Decoder.h"

class LengthFieldBasedFrameDecoder : public Decoder {
public:
    explicit LengthFieldBasedFrameDecoder(size_t maxFrameLength, size_t lengthFieldOffset,
                                          size_t lengthFieldLength, size_t lengthAdjustment,
                                          size_t initialBytesToStrip = 0);

    ~LengthFieldBasedFrameDecoder() override = default;

    Buffer decode(Buffer *buffer) override;

private:
    size_t m_lengthFieldOffset;
    size_t m_lengthFieldLength;
    size_t m_lengthAdjustment;
    size_t m_initialBytesToStrip;
    size_t m_maxFrameLength;
};
