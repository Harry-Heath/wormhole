
#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <map>
#include <vector>

#include "reader_writer.hpp"

struct Node
{
    uint8_t mId;
    Node* mpParent;
    std::map<uint8_t, Node*> mNodes{};

    Node(uint8_t id, Node* parent) 
        : mId(id)
        , mpParent(parent) 
    {
        if (mpParent)
            mpParent->mNodes.insert({mId, this});
    }

    ~Node()
    {
        if (mpParent)
            mpParent->mNodes.erase(mId);
    }

    virtual void read(Reader& reader)
    {
        uint8_t id = reader.readU8();
        if (!reader.success()) return;

        auto it = mNodes.find(id);
        if (it == mNodes.end()) return;

        Node* node = it->second;
        node->read(reader);
    }

    virtual void write(Writer& writer)
    {
        if (mpParent == nullptr) return;

        mpParent->write(writer);
        writer.writeU8(mId);
    }
};


template<typename T>
struct ValueNode : public Node
{
    T mValue{};

    ValueNode(uint8_t id, Node* parent) : Node(id, parent) {}

    void read(Reader& reader) final
    {
        if      constexpr (std::is_same_v<T, uint8_t>)  mValue = reader.readU8();
        else if constexpr (std::is_same_v<T, uint16_t>) mValue = reader.readU16();
        else if constexpr (std::is_same_v<T, uint32_t>) mValue = reader.readU32();
    }

    void write(Writer& writer) final
    {
        assert(mpParent != nullptr);
        
        mpParent->write(writer);
        writer.writeU8(mId);

        if      constexpr (std::is_same_v<T, uint8_t>)  writer.writeU8(mValue);
        else if constexpr (std::is_same_v<T, uint16_t>) writer.writeU16(mValue);
        else if constexpr (std::is_same_v<T, uint32_t>) writer.writeU32(mValue);
    }
};
