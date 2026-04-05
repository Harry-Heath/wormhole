
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

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    virtual void read(Reader& reader)
    {
        uint8_t id{};
        reader.read<uint8_t>(id);
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
        writer.write<uint8_t>(mId);
    }
};


template<typename T>
struct ValueNode : public Node
{
    T mValue{};
    std::function<void(const T&)> mChanged;

    ValueNode(uint8_t id, Node* parent) : Node(id, parent) {}

    void read(Reader& reader) final
    {
        reader.read<T>(mValue);
        if (reader.success() && mChanged) 
            mChanged(mValue);
    }

    void write(Writer& writer) final
    {
        assert(mpParent != nullptr);

        mpParent->write(writer);
        writer.write<uint8_t>(mId);
        writer.write<T>(mValue);
    }
};
