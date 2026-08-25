#include <Glacier/Physics/ZRagdollContainer.h>
#include <Glacier/Physics/CRagdoll2.h>
#include <Glacier/Debug/ZDebugFloat.h>
#include <Glacier/Debug/ZDebugInt.h>


namespace Glacier
{
    static ZDebugFloat g_fNailGun { "nailgun", "Ragdolls will fly through the air and get stuck on walls", -1e38f, 1e38f, 1.0f, nullptr, 0.0f }; // PC at 0x009A33D8
    static ZDebugFloat g_fFlymo { "flymo", "Controls ragdoll animation speed", -1e38f, 1e38f, 1.0f, nullptr, 1.0f }; // PC at 0x007FE5B4
    static ZDebugFloat g_fForceMult { "forcemult", "Will increase impulse power", -1e38f, 1e38f, 1.0f, nullptr, 0.30000001f }; // PC at 0x007FE5B8
    static ZDebugInt g_lTimeOut { "timeout", "Ragdoll timeout before freeze (0 == disable)", 1000, -1000000, 1000000, 1, nullptr }; // PC at 0x007FE5BC

    ZRagdollContainer::ZRagdollContainer()
    {
        m_lMaxNumRagdolls = 4;
        m_pRagdolls = ZUniMemory::NewArray<CRagdoll2>(m_lMaxNumRagdolls, false);
        m_pDragRagdoll = ZUniMemory::New<CRagdoll2>(true);
        m_pUsed = (bool*)ZUniMemory::Allocate(sizeof(bool) * m_lMaxNumRagdolls);
        std::memset(m_pUsed, 0, sizeof(bool) * m_lMaxNumRagdolls);
    }

    ZRagdollContainer::~ZRagdollContainer()
    {
        ZUniMemory::Delete(m_pRagdolls);
        ZUniMemory::Delete(m_pDragRagdoll);
        ZUniMemory::Delete(m_pUsed);
    }

    bool ZRagdollContainer::IsDragdollAvailable() const
    {
        return !m_pDragRagdoll->IsMoving();
    }

    void ZRagdollContainer::DeactivateRagdoll(CRagdoll2* pRagdoll)
    {
        if (pRagdoll == m_pDragRagdoll)
        {
            pRagdoll->Deactivate();
        }
        else
        {
            const int32_t lIndex = static_cast<int32_t>(pRagdoll - m_pRagdolls);
            const int32_t lMaxRagdolls = (g_lMaxRagdolls < 12) ? g_lMaxRagdolls : 12;

            if (lIndex >= 0 && lIndex < lMaxRagdolls)
            {
                // Original assert: i >= 0 && i < m_lMaxNumRagdolls (PS2)
                ZASSERT(lIndex < lMaxRagdolls);
                m_pUsed[lIndex] = false;
            }
        }
    }

    float ZRagdollContainer::GetNailGunValue() const
    {
        return g_fNailGun;
    }

    float ZRagdollContainer::GetForceMultValue() const
    {
        return g_fForceMult;
    }

    float ZRagdollContainer::GetFlymoValue() const
    {
        return g_fFlymo;
    }

    int32_t ZRagdollContainer::GetTimeOutValue() const
    {
        return g_lTimeOut;
    }


    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(int32_t, g_lMaxRagdolls, 0x007FE5C0, 12);
}
