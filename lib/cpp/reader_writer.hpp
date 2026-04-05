
#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>
#include <span>
#include <array>


class Reader
{
    std::span<const uint8_t> mBuffer;
    size_t mIndex = 0;
    bool mError = false;

public:
    Reader(std::span<const uint8_t> buffer) : mBuffer(buffer) {}

    bool error() { return mError; }
    bool success() { return !mError; }
    size_t index() { return mIndex; }

    std::span<const uint8_t> take(size_t count)
    {
        if ((mIndex + count) > mBuffer.size())
            mError = true;

        if (mError)
            return {};

        std::span<const uint8_t> span = mBuffer.subspan(mIndex, count);
        mIndex += count;
        return span;
    }

    template<typename T>
    void read(T& value)
    {
        read_from(*this, value);
    }
};


class Writer
{
    std::span<uint8_t> mBuffer;
    size_t mIndex = 0;
    bool mError = false;

public:
    Writer(std::span<uint8_t> buffer) : mBuffer(buffer) {}

    bool error() { return mError; }
    bool success() { return !mError; }
    size_t index() { return mIndex; }

    std::span<uint8_t> take(size_t count)
    {
        if ((mIndex + count) > mBuffer.size())
            mError = true;

        if (mError)
            return {};

        std::span<uint8_t> span = mBuffer.subspan(mIndex, count);
        mIndex += count;
        return span;
    }

    template<typename T>
    void write(const T& value)
    {
        write_to(*this, value);
    }
};


template<typename T>
static void read_from(Reader& reader, T& value) { read(reader, value); }

template<typename T>
static void write_to(Writer& writer, const T& value) { write(writer, value); }


/// uint8_t
static void read(Reader& reader, uint8_t& value)
{
    auto span = reader.take(1);
    if (span.size() == 0) 
        return;

    value = span[0];
}

static void write(Writer& writer, const uint8_t& value)
{
    auto span = writer.take(1);
    if (span.size() == 0) 
        return;

    span[0] = value;
}


/// uint16_t
static void read(Reader& reader, uint16_t& value)
{
    auto span = reader.take(2);
    if (span.size() == 0) 
        return;

    value = ((uint16_t)span[0] << 8) |
            (uint16_t)span[1];
}

static void write(Writer& writer, const uint16_t& value)
{
    auto span = writer.take(2);
    if (span.size() == 0) 
        return;

    span[0] = (value >> 8) & 0xff;
    span[1] = value & 0xff;
}


/// uint32_t
static void read(Reader& reader, uint32_t& value)
{
    auto span = reader.take(4);
    if (span.size() == 0) 
        return;

    value = ((uint32_t)span[0] << 24) |
            ((uint32_t)span[1] << 16) |
            ((uint32_t)span[2] << 8) |
            (uint32_t)span[3];
}

static void write(Writer& writer, const uint32_t& value)
{
    auto span = writer.take(4);
    if (span.size() == 0) 
        return;

    span[0] = (value >> 24) & 0xff;
    span[1] = (value >> 16) & 0xff;
    span[2] = (value >> 8) & 0xff;
    span[3] = value & 0xff;
}


/// uint64_t
static void read(Reader& reader, uint64_t& value)
{
    auto span = reader.take(8);
    if (span.size() == 0) 
        return;

    value = ((uint64_t)span[0] << 56) |
            ((uint64_t)span[1] << 48) |
            ((uint64_t)span[2] << 40) |
            ((uint64_t)span[3] << 32) |
            ((uint64_t)span[4] << 24) |
            ((uint64_t)span[5] << 16) |
            ((uint64_t)span[6] << 8) |
            (uint64_t)span[7];
}

static void write(Writer& writer, const uint64_t& value)
{
    auto span = writer.take(8);
    if (span.size() == 0) 
        return;

    span[0] = (value >> 56) & 0xff;
    span[1] = (value >> 48) & 0xff;
    span[2] = (value >> 40) & 0xff;
    span[3] = (value >> 32) & 0xff;
    span[4] = (value >> 24) & 0xff;
    span[5] = (value >> 16) & 0xff;
    span[6] = (value >> 8) & 0xff;
    span[7] = value & 0xff;
}


/// float
static void read(Reader& reader, float& value)
{
    union Cast
    {
        float mFloat; 
        uint32_t mInt;
    };

    Cast cast{0.f};
    reader.read(cast.mInt);
    value = cast.mFloat;
}

static void write(Writer& writer, const float& value)
{
    union Cast
    {
        float mFloat; 
        uint32_t mInt;
    };

    Cast cast{value};
    writer.write(cast.mInt);
}


/// double
static void read(Reader& reader, double& value)
{
    union Cast
    {
        double mFloat; 
        uint64_t mInt;
    };

    Cast cast{0.f};
    reader.read(cast.mInt);
    value = cast.mFloat;
}

static void write(Writer& writer, const double& value)
{
    union Cast
    {
        double mFloat; 
        uint64_t mInt;
    };

    Cast cast{value};
    writer.write(cast.mInt);
}


/// std::array
template<typename T, size_t L>
static void read(Reader& reader, std::array<T, L>& value)
{
    for (int i = 0; i < L; i++)
    {
        reader.read(value[i]);
    }
}

template<typename T, size_t L>
static void write(Writer& writer, const std::array<T, L>& value)
{
    for (int i = 0; i < L; i++)
    {
        writer.write(value[i]);
    }
}


/// std::vector
template<typename T>
static void read(Reader& reader, std::vector<T>& value)
{
    uint8_t size{};
    reader.read(size);
    value.resize(size);
    for (int i = 0; i < size; i++)
    {
        reader.read(value[i]);
    }
}

template<typename T>
static void write(Writer& writer, const std::vector<T>& value)
{
    uint8_t size = (uint8_t)value.size();
    writer.write(size);
    for (int i = 0; i < size; i++)
    {
        writer.write(value[i]);
    }
}