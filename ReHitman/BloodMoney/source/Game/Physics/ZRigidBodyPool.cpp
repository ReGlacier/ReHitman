#include <BloodMoney/Game/Physics/ZRigidBodyPool.h>
#include <BloodMoney/Game/Physics/ZRigidBodyPoolManager.h>
#include <BloodMoney/Game/Physics/CRigidBody.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZArray.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniMemory.h>


namespace Hitman
{
    SRBPoolEntry::~SRBPoolEntry()
    {
        // The original inlines ZRigidBodyPool::CleanUp + operator delete here.
        if (kRBPool)
        {
            ZUniMemory::Delete(kRBPool);
            kRBPool = nullptr;
        }
    }

    ZRigidBodyPool::ZRigidBodyPool()
        : m_id(0xFFFF)
        , m_pPool(nullptr)
        , m_pActive(nullptr)
        , m_lCount(0)
        , m_lUnused(0)
        , m_lLast(0)
    {
    }

    ZRigidBodyPool::~ZRigidBodyPool()
    {
        CleanUp();
    }

    bool ZRigidBodyPool::Create(uint32_t total)
    {
        if (m_lCount)
            return false;

        m_lUnused = total;
        m_lCount = total;
        m_lLast = 0;

        m_pPool = total
            ? static_cast<Glacier::ZBaseConRout**>(ZUniMemory::Allocate(sizeof(Glacier::ZBaseConRout*) * total))
            : nullptr;
        m_pActive = total
            ? static_cast<bool*>(ZUniMemory::Allocate(sizeof(bool) * total))
            : nullptr;

        for (uint32_t i = 0; i < total; ++i)
        {
            m_pPool[i] = nullptr;
            m_pActive[i] = false;
        }

        return true;
    }

    void ZRigidBodyPool::CleanUp()
    {
        if (m_pPool)
            ZUniMemory::Free(m_pPool);
        if (m_pActive)
            ZUniMemory::Free(m_pActive);

        m_pPool = nullptr;
        m_pActive = nullptr;
        m_lCount = 0;
        m_lUnused = 0;
        m_lLast = 0;
    }

    uint32_t ZRigidBodyPool::Count() const
    {
        return m_lCount;
    }

    uint32_t ZRigidBodyPool::Unused() const
    {
        return m_lUnused;
    }

    bool ZRigidBodyPool::Assign(Glacier::ZGEOM* pShard)
    {
        ZASSERT(pShard);

        if (!m_lUnused)
            return false;

        while (m_pActive[m_lLast])
        {
            ++m_lLast;
            if (m_lLast == m_lCount)
                m_lLast = 0;
        }

        m_pActive[m_lLast] = true;
        --m_lUnused;

        m_pPool[m_lLast] = pShard->AddEvent(CRigidBody::ClassName);

        CRigidBody* pRigidBody = reinterpret_cast<CRigidBody*>(m_pPool[m_lLast]);
        pRigidBody->m_id = ZRigidBodyPoolManager::Instance().CreateID(m_id, static_cast<uint16_t>(m_lLast));

        return true;
    }

    bool ZRigidBodyPool::Remove(uint16_t index)
    {
        if (index >= m_lCount)
            return false;

        if (m_pActive[index])
        {
            m_pPool[index] = nullptr;
            m_pActive[index] = false;
            ++m_lUnused;
        }

        return true;
    }

    bool ZRigidBodyPool::ForceRemoval(uint16_t total)
    {
        // There must be at least `total` active entries, otherwise nothing happens.
        if (m_lCount - m_lUnused < total)
            return false;

        ZASSERT(ZRigidBodyPoolManager::Exists());

        if (total)
        {
            uint32_t lIndex = m_lLast;

            do
            {
                if (m_pActive[lIndex])
                {
                    CRigidBody* pRigidBody = reinterpret_cast<CRigidBody*>(m_pPool[lIndex]);
                    pRigidBody->Disable();

                    ZASSERT(pRigidBody->m_pBaseGeom);
                    pRigidBody->m_pBaseGeom->MakeInactive();

                    pRigidBody->Delete();

                    if (lIndex < m_lCount && m_pActive[lIndex])
                    {
                        m_pPool[lIndex] = nullptr;
                        m_pActive[lIndex] = false;
                        ++m_lUnused;
                    }

                    --total;
                }

                if (++lIndex == m_lCount)
                    lIndex = 0;
            }
            while (total);
        }

        return true;
    }

    bool ZRigidBodyPool::Assign(const Glacier::ZGROUP* pShards, uint32_t lAssigned, Glacier::ZArray<Glacier::ZGEOM*>* pAssigned,
                                Glacier::ZArray<Glacier::ZGEOM*>* pLeftovers, bool bForced)
    {
        ZASSERT(pShards);

        if (m_lUnused < lAssigned)
        {
            if (!bForced)
                return false;

            if (!ForceRemoval(static_cast<uint16_t>(lAssigned - static_cast<uint16_t>(m_lUnused))))
                return false;
        }

        // Collect every child that does not already own a rigid body event.
        Glacier::ZArray<Glacier::ZGEOM*> aShards;
        for (Glacier::ZBaseGeom* i = pShards->m_pGroupFirst; i; i = i->Next())
        {
            Glacier::ZGEOM* pGeom = i->GetGeom();
            if (!pGeom->FindEvent(CRigidBody::Name) && !pGeom->FindEvent(CRigidBody::ClassName))
                aShards.Add(pGeom);
        }

        if (lAssigned < aShards.Count())
        {
            // Pick random shards until enough were assigned, then hand the rest over as leftovers.
            while (lAssigned)
            {
                const uint32_t lIdx = static_cast<uint32_t>(Glacier::RandomRange(0, static_cast<int>(aShards.Count()) - 1));
                Glacier::ZGEOM* pShard = aShards[lIdx];

                Assign(pShard);
                if (pAssigned)
                    pAssigned->Add(pShard);
                aShards.RemoveByIdx(lIdx);

                --lAssigned;
            }

            if (pLeftovers)
            {
                for (uint32_t j = 0; j < aShards.Count(); ++j)
                    pLeftovers->Add(aShards[j]);
            }
        }
        else
        {
            // Fewer shards than requested: assign all of them.
            for (uint32_t j = 0; j < aShards.Count(); ++j)
            {
                Glacier::ZGEOM* pShard = aShards[j];
                Assign(pShard);
                if (pAssigned)
                    pAssigned->Add(pShard);
            }
        }

        return true;
    }
}
