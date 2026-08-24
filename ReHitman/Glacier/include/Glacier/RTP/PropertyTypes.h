#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


#pragma push_macro("small")
#undef small


namespace Glacier
{
    struct ZGEOMREF
    {
        uint32_t m_Value{};
    };
    RE_VERIFY_SIZE(ZGEOMREF, 0x4);

    struct ZANIM : ZRTString
    {
    };
    RE_VERIFY_SIZE(ZANIM, 0x4);

    struct ZCOLOR
    {
        uint32_t m_Value{};
    };
    RE_VERIFY_SIZE(ZCOLOR, 0x4);

    struct ZFILENAME : ZRTString
    {
    };
    RE_VERIFY_SIZE(ZFILENAME, 0x4);

    struct ZRawData
    {
        // methods
        ZRawData() = default;
        ~ZRawData()
        {
            if (m_Data && m_Size)
            {
                ZUniMemory::Delete(m_Data);
                m_Data = nullptr;
            }
        }

        void SetSize(uint32_t size)
        {
            m_Size = size;
        }

        // members
        void* m_Data{};
        uint32_t m_Size{};
    };
    RE_VERIFY_SIZE(ZRawData, 0x8);

    enum ESecurityZone : uint32_t
    {
        eZoneUNDEFINED = 0,
        eZone1 = 1,
        eZone2 = 2,
        eZone2A = 4,
        eZone2B = 8,
        eZone3 = 16,
        eZone3A = 32,
        eZone3B = 64,
        eZoneMegaForbidden = 128,
    };

    enum EAnimMode : uint32_t
    {
        none = 0,
        scale = 1,
        jump = 2,
        alpha = 4,
        saturation = 8,
        big = 16,
        small = 32,
    };
}

#pragma pop_macro("small")
