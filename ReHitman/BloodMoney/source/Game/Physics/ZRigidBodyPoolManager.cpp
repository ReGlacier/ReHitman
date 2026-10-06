#include <BloodMoney/Game/Physics/ZRigidBodyPoolManager.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Hitman
{
    // PC: dword_7FE60C == 3 (engine/fysix/rigidbodypool.cpp).
    uint32_t ZRigidBodyPoolManager::m_lMaxEntries = 3;

    ZRigidBodyPoolManager::ZRigidBodyPoolManager()
        : m_pEntries(nullptr)
        , m_lCount(0)
    {
    }

    ZRigidBodyPoolManager::~ZRigidBodyPoolManager()
    {
        if (m_pEntries)
        {
            // The original destroys the array with the cookie stored at m_pEntries[-4];
            // here the array was allocated with m_lMaxEntries elements.
            ZUniMemory::DeleteArray(m_pEntries, m_lMaxEntries);
            m_pEntries = nullptr;
        }
    }

    ZRigidBodyPool* ZRigidBodyPoolManager::Add(const char* pszName)
    {
        ZASSERT(std::strlen(pszName) < sizeof(SRBPoolEntry::sName));
        ZASSERT(m_lCount < m_lMaxEntries);

        if (m_pEntries)
        {
            if (ZRigidBodyPool* pExistingPool = Find(pszName))
                return pExistingPool;
        }
        else
        {
            m_pEntries = ZUniMemory::NewArray<SRBPoolEntry>(m_lMaxEntries);
        }

        std::strcpy(m_pEntries[m_lCount].sName, pszName);

        ZRigidBodyPool* pPool = ZUniMemory::New<ZRigidBodyPool>();
        m_pEntries[m_lCount].kRBPool = pPool;
        pPool->m_id = static_cast<uint16_t>(m_lCount);
        ++m_lCount;

        return pPool;
    }

    ZRigidBodyPool* ZRigidBodyPoolManager::Find(const char* pszName) const
    {
        const uint32_t lCount = m_lCount;
        uint32_t lIndex = 0;

        if (lCount)
        {
            // Walk the entry array until the first entry without a pool.
            for (const SRBPoolEntry* pEntry = m_pEntries; pEntry->kRBPool; ++pEntry)
            {
                if (!std::strcmp(pEntry->sName, pszName))
                    return m_pEntries[lIndex].kRBPool;

                if (++lIndex >= lCount)
                    return nullptr;
            }
        }

        return nullptr;
    }

    bool ZRigidBodyPoolManager::Remove(uint16_t id)
    {
        const uint16_t wPoolIndex = static_cast<uint16_t>(id >> 12);
        if (wPoolIndex >= m_lCount)
            return false;

        ZRigidBodyPool* pPool = m_pEntries[wPoolIndex].kRBPool;

        const uint16_t wBodyIndex = static_cast<uint16_t>(id & BODY_ID_MAX);
        if (wBodyIndex >= pPool->m_lCount)
            return false;

        if (pPool->m_pActive[wBodyIndex])
        {
            pPool->m_pPool[wBodyIndex] = nullptr;
            pPool->m_pActive[wBodyIndex] = false;
            ++pPool->m_lUnused;
        }

        return true;
    }

    uint16_t ZRigidBodyPoolManager::CreateID(uint16_t wPoolId, uint16_t wBodyId) const
    {
        ZASSERT(wPoolId <= POOL_ID_MAX);
        ZASSERT(wBodyId <= BODY_ID_MAX);

        return static_cast<uint16_t>((wPoolId << 12) | wBodyId);
    }
}

// Singleton instance definition. The `m_pInstance` member belongs to the Glacier class template,
// so (unlike the rest of the file) it is spelled out inside the Glacier namespace.
namespace Glacier
{
    template <>
    Hitman::ZRigidBodyPoolManager* ZComponentSingleton<Hitman::ZRigidBodyPoolManager, ZRuntimeComponentBase>::m_pInstance = nullptr;
}
