#include <Glacier/Data/ZStaticGameLevelData.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    STATIC_CLASS_VAR_IMPL(ZStaticGameLevelData, ZStaticGameLevelData*, m_Instance, 0x008BE14C, nullptr);

    void ZStaticGameLevelData::Create()
    {
        ZASSERT(m_Instance == nullptr);

        auto* instance = static_cast<ZStaticGameLevelData*>(ZUniMemory::Allocate(sizeof(ZStaticGameLevelData)));
        if (!instance)
        {
            m_Instance = nullptr;
            return;
        }

        instance->m_pFirstDataBlock = nullptr;
        m_Instance = instance;
    }

    ZStaticGameLevelData* ZStaticGameLevelData::Instance()
    {
        return m_Instance;
    }

    void ZStaticGameLevelData::Load(const void* pData)
    {
        m_pFirstDataBlock = pData;
    }

    void ZStaticGameLevelData::Destroy()
    {
        if (m_pFirstDataBlock)
        {
            ZUniMemory::Free(const_cast<void*>(m_pFirstDataBlock));
            m_pFirstDataBlock = nullptr;
        }

        ZUniMemory::Free(this);
        m_Instance = nullptr;
    }

}
