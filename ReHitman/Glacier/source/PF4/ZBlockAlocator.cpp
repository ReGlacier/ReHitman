#include <Glacier/PF4/ZData.h>
#include <Glacier/ZUniAssert.h>

#include <cstdint>


namespace Glacier::PF4
{
    // ZBlockAlocator (PS2 build: 0x1F2FAC Init, 0x1F3054 Alloc, 0x1F30EC BelongsTo, 0x1F3178 Free)

    // The PS2 ctor is empty; ZData fills the fields before calling Init.
    ZBlockAlocator::ZBlockAlocator() = default;

    ZBlockAlocator::~ZBlockAlocator() = default;

    void ZBlockAlocator::Init(int16_t* pStack, ZDataRef* pDataRef)
    {
        m_Data = pDataRef;
        m_Stack = pStack;
        m_Count = m_MaxBlocks;
        for (int i = 0; i < m_MaxBlocks; ++i)
        {
            m_Stack[i] = static_cast<int16_t>(i);
        }
    }

    ZDataRef* ZBlockAlocator::Alloc()
    {
        if (m_Count <= 0)
        {
            return nullptr;
        }

        --m_Count;
        return m_Data + static_cast<int>(m_Stack[m_Count]) * m_BlockSize;
    }

    void ZBlockAlocator::Free(ZDataRef* pRef)
    {
        ZASSERT(m_BlockSize > 0);

        const auto uOffset = static_cast<uintptr_t>(reinterpret_cast<intptr_t>(pRef))
            - static_cast<uintptr_t>(reinterpret_cast<intptr_t>(m_Data));
        const int iBlock = static_cast<int>(uOffset / (12u * static_cast<unsigned>(m_BlockSize)));

        ZASSERT(m_Count < m_MaxBlocks);
        ZASSERT(iBlock >= 0 && iBlock < m_MaxBlocks);

        m_Stack[m_Count++] = static_cast<int16_t>(iBlock);
    }

    bool ZBlockAlocator::BelongsTo(ZDataRef* pDataRef) const
    {
        const auto uAddr = static_cast<uintptr_t>(reinterpret_cast<intptr_t>(pDataRef));
        const auto uBase = static_cast<uintptr_t>(reinterpret_cast<intptr_t>(m_Data));
        return uAddr >= uBase && uAddr < uBase + 12u * static_cast<unsigned>(m_BlockSize * m_MaxBlocks);
    }
}
