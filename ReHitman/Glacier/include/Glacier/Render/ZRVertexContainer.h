#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/System/ZRX86AllocIf.h>
#include <cstdint>


namespace Glacier
{
    class ZRVertexContainer
    {
    public:
        // methods
        ZRVertexContainer();
        uint32_t Create(uint32_t lNumVertices, uint32_t lVertexSize, ZRX86AllocIf* pAllocator, uint32_t lAlignment);
        void Release();

        // members
        uint32_t m_lNumVertices{0};
        uint32_t m_lVertexOffset{0};
        uint32_t m_lVertexSize{0};
        uint32_t m_lAllocateOffset{0};
        uint32_t m_lAllocateSize{0};
        ZRX86AllocIf* m_pAllocator{nullptr};
    };
    RE_VERIFY_OFFSET(ZRVertexContainer, m_pAllocator, 0x14); // Verified PC
}