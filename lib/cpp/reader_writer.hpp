
#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>
#include <span>


class Reader
{
    std::span<const uint8_t> mBuffer;
    size_t mIndex = 0;
    bool mSuccess = true;

public:
    Reader(std::span<const uint8_t> buffer) : mBuffer(buffer) {}

    bool success() { return mSuccess; }
    size_t index() { return mIndex; }

private:
    std::span<const uint8_t> take(size_t count)
    {
        if ((mIndex + count) > mBuffer.size())
        {
            mSuccess = false;
            return {};
        }

        std::span<const uint8_t> span = mBuffer.subspan(mIndex, count);
        mIndex += count;
        return span;
    }

public:
    uint8_t readU8()
    {
        auto span = take(1);
        if (span.size() == 0) 
            return {};

        return span[0];
    }

    uint16_t readU16()
    {
        auto span = take(2);
        if (span.size() == 0) 
            return {};

        return ((uint16_t)span[0] << 8) |
                (uint16_t)span[1];
    }

    uint32_t readU32()
    {
        auto span = take(4);
        if (span.size() == 0) 
            return {};

        return ((uint32_t)span[0] << 24) |
                ((uint32_t)span[1] << 16) |
                ((uint32_t)span[2] << 8) |
                (uint32_t)span[3];
    }
};


class Writer
{
    std::span<uint8_t> mBuffer;
    size_t mIndex = 0;
    bool mSuccess = true;

public:
    Writer(std::span<uint8_t> buffer) : mBuffer(buffer) {}

    bool success() { return mSuccess; }
    size_t index() { return mIndex; }

private:
    std::span<uint8_t> take(size_t count)
    {
        if ((mIndex + count) > mBuffer.size())
        {
            mSuccess = false;
            return {};
        }

        std::span<uint8_t> span = mBuffer.subspan(mIndex, count);
        mIndex += count;
        return span;
    }

public:
    void writeU8(const uint8_t& value)
    {
        auto span = take(1);
        if (span.size() == 0) 
            return;

        span[0] = value;
    }

    void writeU16(const uint16_t& value)
    {
        auto span = take(2);
        if (span.size() == 0) 
            return;

        span[0] = (value >> 8) & 0xff;
        span[1] = value & 0xff;
    }

    void writeU32(const uint32_t& value)
    {
        auto span = take(4);
        if (span.size() == 0) 
            return;

        span[0] = (value >> 24) & 0xff;
        span[1] = (value >> 16) & 0xff;
        span[2] = (value >> 8) & 0xff;
        span[3] = value & 0xff;
    }
};



