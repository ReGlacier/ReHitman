#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Physics/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRagdollContainer
    {
    public:
        // methods
        ZRagdollContainer();
        ~ZRagdollContainer();

        CRagdoll2* GetRagdoll(bool);
        CRagdoll2* NumToPtr(int32_t lRagdollNum);
        int32_t PtrToNum(CRagdoll2* pRagdoll);
        uint32_t NumberOfAvailableRagdolls() const;
        bool IsDragdollAvailable() const;
        void DeactivateRagdoll(CRagdoll2* pRagdoll);
        float GetNailGunValue() const;
        float GetForceMultValue() const;
        float GetFlymoValue() const;
        int32_t GetTimeOutValue() const;

        // members
        CRagdoll2* m_pRagdolls; // +0x0
        CRagdoll2* m_pDragRagdoll; // +0x4
        int32_t m_lMaxNumRagdolls; // +0x8
        bool* m_pUsed; // +0xC
    };

    STATIC_GLOBAL_CLASS_INSTANCE(int32_t, g_lMaxRagdolls);
}