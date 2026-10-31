#pragma once

namespace gill_editor {

    enum class Type : uint32_t {
        Gizmo = 1, //001
        Entity = 2, // 010
        Bone = 3, //011
        //we read the top 3 bits and compare it to this
    };

   inline Type& operator++(Type& t) {
        uint32_t i = static_cast<uint32_t>(t);
        i++;
        t = static_cast<Type>(i);
        return t;
    }

   inline Type& operator+(Type& t, uint32_t i) {
       uint32_t j = static_cast<uint32_t>(t);
       j = j + i ;
       t = static_cast<Type>(j);
       return t;
   }

    struct Pixel {
       Type type;
       uint32_t payload;
   };


    inline uint32_t Encode(const Type& type, uint32_t index) {
        uint32_t id;
        if (index >= 536870911) {
            std::cerr<<"Index Reached the Max Value" <<std::endl;
            return index;
        }
        switch (type) {
            case Type::Gizmo:
                id = static_cast<uint32_t>(Type::Gizmo);
                id = id << 29;
                id = id | index;
                break;
            case Type::Entity:
                id = static_cast<uint32_t>(Type::Entity);
                id = id<<29;
                id = id | index;
                break;
            case Type::Bone:
                id = static_cast<uint32_t>(Type::Bone);
                id = id<<29;
                id = id | index;
                break;
            default:
                id = index;
        }
        return id;
    }

    inline Pixel Decode(uint32_t pixel) {
         Pixel p;
        uint32_t kind = pixel>>29;
        uint32_t payload = pixel & (1u << 29) - 1;
        p.type = static_cast<Type>(kind);
        p.payload = payload;
        return p;
    }
}