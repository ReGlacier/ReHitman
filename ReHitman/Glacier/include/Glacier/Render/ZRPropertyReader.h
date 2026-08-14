#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRPropertyReader
    {
    public:
        // types
        struct SProperty
        {
            uint32_t lName;
            uint32_t lData;
            uint32_t lSize;
            uint32_t lType;
        };

        enum PROPERTY_TYPE
        {
            PT_FLOAT = 0x0,
            PT_CHAR = 0x1,
            PT_UINT32 = 0x2,
            PT_LIST = 0x3
        };

        // vtbl
        // methods
        ZRPropertyReader();
        void Dump(int, const char*);
        void GetNamedListElement(uint32_t lName, ZRPropertyReader& outElement) const;

        // members
        IBuffer* m_pBuffer { nullptr };
        SProperty* m_pProperty { nullptr };
    };
}