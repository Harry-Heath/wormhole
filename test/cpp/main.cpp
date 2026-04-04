
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <variant>

#include <state.hpp>

// enum class Type
// {
//     U8, U16, U32,
//     S8, S16, S32,
//     F32, F64,
    
//     ENUM,
//     ARRAY, 
//     STRING,
//     STRUCT,
// };

// struct EnumValue
// {
//     std::string mName;
//     uint8_t mValue;
// };

// enum class Access
// {
//     READ,
//     WRITE,
//     READ_WRITE,
// };


// struct Field
// {
//     std::string mName;
//     Type mType;
//     uint32_t mArrayLength;
//     Type mArrayType;
//     std::vector<Field> mStructFields;
//     std::vector<EnumValue> mEnumValues;
// };

// struct PropertyField;
// struct PropertyArray;
// using Property = std::variant<PropertyField, PropertyArray>;


// struct PropertyField
// {
//     uint16_t mId;
//     std::string mName;

//     Type mType;
//     Access mAccess = Access::READ;
//     uint32_t mArrayLength; // Array length of 0 is dynamic
//     Type mArrayType;
//     std::vector<Field> mStructFields;
//     std::vector<EnumValue> mEnumValues;
// };

// struct PropertyArray
// {
//     uint16_t mId;
//     std::string mName;

//     uint32_t mArrayLength; // Array length of 0 is dynamic
//     std::vector<Property> mProperties;
// };

// const std::vector<Property> PROPERTIES = {
//     PropertyArray{
//         .mId = 1,
//         .mName = "sensors",
//         .mProperties = {
//             PropertyField{
//                 .mId = 1,
//                 .mName = "resolution",
//                 .mType = Type::STRUCT,
//                 .mAccess = Access::READ_WRITE,
//                 .mStructFields = {
//                     Field{"width", Type::U16},
//                     Field{"height", Type::U16},
//                 },
//             },
//             PropertyField{
//                 .mId = 2,
//                 .mName = "zoom_level",
//                 .mType = Type::U8,
//                 .mAccess = Access::READ_WRITE,
//             },
//             PropertyField{
//                 .mId = 3,
//                 .mName = "video_format",
//                 .mType = Type::ENUM,
//                 .mAccess = Access::READ_WRITE,
//                 .mEnumValues = {
//                     EnumValue{"h264", 0},
//                     EnumValue{"hevc", 1},
//                     EnumValue{"mjpg", 2},
//                 },
//             },
//         },
//     },
//     PropertyField{
//         .mId = 2,
//         .mName = "controls.velocity_demand",
//         .mType = Type::STRUCT,
//         .mAccess = Access::READ_WRITE,
//         .mStructFields = {
//             Field{"x", Type::F32},
//             Field{"y", Type::F32},
//         },
//     },
// };


struct ExamplePropertyGroup
{
    Node mNode;
    PropertyRoot& mRoot;

    ExamplePropertyGroup(uint8_t id, Node& parent, PropertyRoot& root)
        : mNode{id, &parent}
        , mRoot{root}
    {
    }

    Property<uint8_t> some_value{1, mNode, mRoot};
    Property<uint16_t> some_other_value{2, mNode, mRoot};
};

struct ExampleProperties
{
    PropertyRoot mRoot{};
    Node& mNode = mRoot.mNode;
    
    
    struct Controls
    {
        Node mNode;
        PropertyRoot& mRoot;
        
        Controls(uint8_t id, Node& parent, PropertyRoot& root)
            : mNode{id, &parent}
            , mRoot{root}
        {
        }

        Property<uint8_t> velocity_x{1, mNode, mRoot};
        Property<uint8_t> velocity_y{2, mNode, mRoot};
    };

    Controls controls{1, mNode, mRoot};
    PropertyArray<ExamplePropertyGroup> sensors{2, mNode, mRoot};
    PropertyArray<Property<uint16_t>> some_array{3, mNode, mRoot};
};


int main()
{

    ExampleProperties properties{};
    properties.controls.velocity_x.set(3);

    printf("%ld\n", properties.mRoot.mQueue.size());

    for (const Packet& packet : properties.mRoot.mQueue)
    {
        printf("{ ", packet.mLength);
        for (int i = 0; i < packet.mLength; i++)
        {
            printf("%02X ", packet.mpData[i]);
        }
        printf("}\n");
    }

    Packet test{.mLength = 3, .mpData = {1, 1, 4}};
    properties.mRoot.receive(test);

    printf("%d\n", properties.controls.velocity_x.value());


    // PROPERTIES[0].index();
    // Properties properties{};
    // properties.sensors[0].resolution.set({1920, 1080});
    // uint8_t zoom = properties.sensors[1].zoom_level.get();
    // properties.controls.velocity_demand.fetch();
    

    return 0;
}