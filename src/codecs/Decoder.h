#pragma once

#include "base/Buffer.h"

class Decoder {
public:
    Decoder() = default;
    virtual ~Decoder() = default;

    virtual Buffer decode(Buffer *buffer) = 0;
};
