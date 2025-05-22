#pragma once

#include "base/Buffer.h"

class Decoder {
public:
    Decoder() = default;

    virtual ~Decoder() = default;

    virtual BufferPtr decode(Buffer *buffer) = 0;
};
