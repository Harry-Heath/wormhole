
#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>
#include <span>
#include <functional>

#include "nodes.hpp"

struct Packet
{
    static constexpr uint8_t MAX_SIZE = 32;
    
    uint8_t mLength{};
    uint8_t mpData[MAX_SIZE]{};

    uint8_t* begin() { return mpData; }
    uint8_t* end() { return mpData + mLength; }

    const uint8_t* begin() const { return mpData; }
    const uint8_t* end() const { return mpData + mLength; }
};


using Queue = std::vector<Packet>;

struct PropertyRoot
{
    Node mNode;
    Queue mQueue{};

    PropertyRoot() : mNode(0, nullptr) {}
    
    void receive(const Packet& packet)
    {
        Reader reader{packet};
        mNode.read(reader);
    }

    template<typename Func>
    void send(Func&& func)
    {
        for (Packet& packet : mQueue)
        {
            func(packet);
        }
        mQueue.clear();
    }
};

template<typename T>
struct Property
{
    ValueNode<T> mNode;
    PropertyRoot& mRoot;

    Property(uint8_t id, Node& parent, PropertyRoot& root) 
        : mNode{id, &parent}
        , mRoot{root}
    {
    }
    
    void set(const T& value)
    {
        mNode.mValue = value;

        Packet packet{Packet::MAX_SIZE};

        Writer writer{packet};
        mNode.write(writer);

        packet.mLength = writer.index();
        
        if (writer.success())
            mRoot.mQueue.push_back(packet);
    }

    const T& value()
    {
        return mNode.mValue;
    }
};


template<typename T>
struct PropertyArray
{
    Node mNode;
    PropertyRoot& mRoot;
    std::vector<T> mProperties{};

    Property<uint8_t> mSize{0, mNode, mRoot};

    PropertyArray(uint8_t id, Node& parent, PropertyRoot& root)
        : mNode{id, &parent}
        , mRoot{root}
    {
    }
};






