#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Debug/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class AttackerPosManager
    {
    public:
        // methods
        AttackerPosManager();

        void Reset();
        void SetTarget(ZGEOM* pTargetGeom);
        int32_t GetSliceIndex(ZGEOM* pGeom);
        int32_t AddAttacker(ZGEOM* pGeom);
        int32_t GetBetterSlice(int32_t lSlice);
        bool GetBetterAttackPos(ZREF rActor, ZVector3& vPos);
        void DrawDebugObjects(ZDrawDebugRender* pRender);

        // members
        int32_t m_aSlices[12];
        float m_fDeltaSlice;
        ZVector3 m_vTargetPos;
    };

    RE_VERIFY_OFFSET(AttackerPosManager, m_vTargetPos, 0x34); // Verified by AttackerPosManager::AddAttacker (PC)
}
