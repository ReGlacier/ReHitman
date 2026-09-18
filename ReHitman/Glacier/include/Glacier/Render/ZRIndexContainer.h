#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/System/ZRX86AllocIf.h>
#include <cstdint>


namespace Glacier
{
    struct ZRIndexSubRange
    {
        uint32_t m_lIndexCount;
        uint32_t m_lIndexOffset;
    };

    class ZRIndexContainer
    {
    public:
        // methods
        ZRIndexContainer();
        void Create(const uint16_t* pPackedData, ZRX86AllocIf* pAllocator, uint32_t lFlags);
        void Release();

        // members
        uint32_t m_lSubRangeCount{0};
        uint32_t m_lTotalIndexCount{0};
        uint32_t m_lFlags{0};          
        ZRIndexSubRange* m_pSubRanges{nullptr};
        ZRX86AllocIf* m_pAllocator{nullptr};
        ZRIndexSubRange m_InlineSubRange;     
    };
}