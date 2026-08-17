#include <Glacier/Fysix/ZBoneExtendPool.h>
#include <Glacier/Fysix/ZBoneExtend.h>


namespace Glacier
{
    ZBoneExtendPool::ZBoneExtendPool() = default;

    ZBoneExtendPool::~ZBoneExtendPool()
    {
        if (m_pBXPool && m_wCount)
        {
            for (int i = 0; i < m_wCount; ++i)
            {
                (&m_pBXPool[i])->~ZBoneExtend();
            }

            ZUniMemory::Free(m_pBXPool);
            m_pBXPool = nullptr;
            m_wCount = 0;
        }
    }
    
    bool ZBoneExtendPool::Create(uint16_t total)
    {
        // Cleanup prev
        if (m_pBXPool && m_wCount)
        {
            for (int i = 0; i < m_wCount; ++i)
            {
                (&m_pBXPool[i])->~ZBoneExtend();
            }

            ZUniMemory::Free(m_pBXPool);
            m_pBXPool = nullptr;
            m_wCount = 0;
        }

        // Allocate new
        m_pBXPool = nullptr;
        m_wCount = total;
        
        if (m_wCount)
        {
            m_pBXPool = (ZBoneExtend*)ZUniMemory::Allocate(sizeof(ZBoneExtend) * total);
            for (int i = 0; i < m_wCount; ++i)
            {
                new (&m_pBXPool[i]) ZBoneExtend();
            }
        }
        
        return true;
    }

    ZBoneExtend& ZBoneExtendPool::operator[](uint16_t index)
    {
        return m_pBXPool[index];
    }

    template<>
    ZBoneExtendPool* ZComponentSingleton<ZBoneExtendPool, ZRuntimeComponentBase>::m_pInstance = nullptr;
}