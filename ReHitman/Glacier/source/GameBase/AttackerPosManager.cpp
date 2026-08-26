#include <Glacier/GameBase/AttackerPosManager.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Glacier
{
    AttackerPosManager::AttackerPosManager()
    {
        m_fDeltaSlice = 0.52359873f;
    }

    void AttackerPosManager::Reset()
    {
        for (auto& slice : m_aSlices)
            slice = 0;
    }

    void AttackerPosManager::SetTarget(ZGEOM* pTargetGeom)
    {
        vreset(m_vTargetPos);
        pTargetGeom->GetRootPoint(m_vTargetPos);
    }

    int32_t AttackerPosManager::GetSliceIndex(ZGEOM* pGeom)
    {
        ZVector3 vPos;
        pGeom->GetRootPoint(vPos);
        vsub(vPos, vPos, m_vTargetPos);
        return static_cast<int32_t>(GetAngle(vPos.x, vPos.z) / m_fDeltaSlice);
    }

    int32_t AttackerPosManager::AddAttacker(ZGEOM* pGeom)
    {
        return GetSliceIndex(pGeom);
    }

    int32_t AttackerPosManager::GetBetterSlice(int32_t lSlice)
    {
        const int32_t currentCount = m_aSlices[lSlice];

        int32_t rightSlice = lSlice;
        int32_t leftSlice  = lSlice;

        for (int32_t offset = 1; offset < 3; ++offset)
        {
            rightSlice = (rightSlice + 1) % 12;
            leftSlice = (leftSlice - 1 < 0) ? 11 : (leftSlice - 1);

            const int32_t rightCount = m_aSlices[rightSlice];
            const int32_t leftCount  = m_aSlices[leftSlice];

            if (rightCount < currentCount && rightCount <= leftCount)
            {
                return +offset;
            }

            if (leftCount < currentCount && leftCount <= rightCount)
            {
                return -offset;
            }
        }

        return lSlice;
    }

    bool AttackerPosManager::GetBetterAttackPos(ZREF rActor, ZVector3& vPos)
    {
        auto* pActor = ZGEOM::RefToPtr(rActor);
        auto lAttackerSlice = AddAttacker(pActor);
        auto lBetterSlice = GetBetterSlice(lAttackerSlice);

        if (lAttackerSlice == lBetterSlice)
            return false;

        // TODO: Finish me after PF4 reversed
        return false;
    }

    void AttackerPosManager::DrawDebugObjects(ZDrawDebugRender* pRender)
    {
        // Need restore this somehow...
    }
}
