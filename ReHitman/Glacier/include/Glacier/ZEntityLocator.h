#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZMath.h>

namespace Glacier {
    class ZEntityLocator
    {
    public:
        char pad_0000[36]; //0x0000
        Vector3 position; //0x0024
        char pad_0030[32]; //0x0030
        ZEntityLocator* parent; //0x0050
        char pad_0054[8]; //0x0054
        uint32_t m_unk; //0x005C
        void* m_data; //0x0060
        char pad_0064[8]; //0x0064
        char* entityName; //0x006C
    }; //Size: 0x0080
}