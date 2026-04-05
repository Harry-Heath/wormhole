
#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>
#include <span>
#include <functional>
#include <memory>

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

struct Object
{
    Node mNode;
    Object& mRoot;

    Queue mQueue{};

    Object() 
        : mNode{0, nullptr}
        , mRoot{*this} 
    {
    }

    Object(uint8_t id, Node& parent, Object& root) 
        : mNode{id, &parent}
        , mRoot{root.mRoot} 
    {
    }
    
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
    Object& mRoot;
    std::function<void(const T&)> mChanged;

    Property(uint8_t id, Node& parent, Object& root) 
        : mNode{id, &parent}
        , mRoot{root.mRoot}
    {
        mNode.mChanged = [this](const T& value){
            if (this->mChanged)
                this->mChanged(value);
        };
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

    const T& value() const
    {
        return mNode.mValue;
    }
};


template<typename T>
struct PropertyArray
{
    Node mNode;
    Object& mRoot;
    std::vector<std::unique_ptr<T>> mProperties{};

    Property<uint8_t> mSize{UINT8_MAX, mNode, mRoot};

    PropertyArray(uint8_t id, Node& parent, Object& root)
        : mNode{id, &parent}
        , mRoot{root.mRoot}
    {
        mSize.mNode.mChanged = [this](uint8_t size){
            this->resizeArray(size);
            // TODO: Event, on changed
        };
    }

    void resizeArray(uint8_t size)
    {
        mProperties.clear();
        for (uint8_t i = 0; i < size; i++)
        {
            mProperties.push_back(std::make_unique<T>(i, mNode, mRoot));
        }
    }

    void resize(uint8_t size)
    {
        resizeArray(size);
        mSize.set(size);
    }

    uint8_t size() const { return (uint8_t)mProperties.size(); }

    T& operator[](uint8_t index)
    {
        return *mProperties[index];
    } 

    const T& operator[](uint8_t index) const
    {
        return *mProperties[index];
    }
};
