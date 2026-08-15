#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZFixedArray.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZLNKOBJ;
    class ZPrimAccess;

    class ZDecalMarkControllerBase
    {
    public:
        // types
        struct ZBonesDecal
        {
            ZLNKOBJ* m_pLnkObj;
            ZVector3 m_vPosition;
            ZVector3 m_vDirection;
            float m_fRadius;
            float m_fRandomAngle;
            uint32_t m_lBoneId;
            uint32_t m_lSourcePrim;
            ZPrimAccess* m_pPrimAccess[4];
            ZDecalMarkControllerBase::ZBonesDecal* m_pNextSameLookup;
            ZDecalMarkControllerBase::ZBonesDecal* m_pNext;
            ZDecalMarkControllerBase::ZBonesDecal* m_pPrev;
        };

        // methods
        ZDecalMarkControllerBase();
        ZDecalMarkControllerBase::ZBonesDecal* GetAvailBoneDecal();
        ZDecalMarkControllerBase::ZBonesDecal* GetBoneDecals(ZLNKOBJ* pLnkObj);
        void RemoveBoneDecalBaseGeom(ZLNKOBJ* pLnkObj);
        void RemoveBoneDecal(ZLNKOBJ* pLnkObj, ZDecalMarkControllerBase::ZBonesDecal* pBonesDecal);
        void AddBoneDecal(ZLNKOBJ* pLnkObj, const float* pvPosition, const float* pvDirection, float fRadius, uint32_t lBoneId, uint32_t lSourcePrim);

        // members
        ZDecalMarkControllerBase::ZBonesDecal* m_pBonesFirst;
        ZDecalMarkControllerBase::ZBonesDecal* m_pBonesLast;
        ZDecalMarkControllerBase::ZBonesDecal* m_pBoneDecalsLookup[256];
        ZFixedArray<ZDecalMarkControllerBase::ZBonesDecal, 128> m_BoneDecals; // PS2 - 128, XBox - 1024, PC - 128
    };
    RE_VERIFY_SIZE(ZDecalMarkControllerBase::ZBonesDecal, 0x48); // Approved by PC ctor (72 byte stride)
    RE_VERIFY_SIZE(ZDecalMarkControllerBase, 0x280C); // Manually verified
}