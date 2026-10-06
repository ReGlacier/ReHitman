#include <Glacier/GameBase/Boid/ZBoidSystem.h>

#include <Glacier/GameBase/Boid/ZBoid.h>
#include <Glacier/System/ZSysInterface.h>
#include <cstddef>


namespace Glacier
{
    ZBoidSystem::ZBoidSystem(PF4::ZInterface* pPathFinder)
        : m_BoidsPool()
        , m_pPF4Interface(pPathFinder)
        , m_iNextBoidID(0)
    {
    }

    int ZBoidSystem::GetNextBoidID()
    {
        return m_iNextBoidID++;
    }

    ZBoid* ZBoidSystem::AddBoid(ZBoid* pBoid)
    {
        m_BoidsPool.push_back(pBoid);
        return pBoid;
    }

    void ZBoidSystem::FrameUpdate()
    {
        const float fDeltaTime = g_pSysInterface->DeltaFrameTime;
        if (fDeltaTime == 0.0f)
            return;

        const size_t lCount = m_BoidsPool.size();
        for (size_t i = 0; i < lCount; ++i)
            m_BoidsPool[i]->FrameUpdate(m_BoidsPool, fDeltaTime);

        for (size_t i = 0; i < lCount; ++i)
            m_BoidsPool[i]->Move(m_BoidsPool, fDeltaTime);
    }
}
