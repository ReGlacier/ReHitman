#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/CQuadtree.h>

namespace Glacier
{
    class ZEntityLocator
    {
    public:
        // Data
        ZMat3x3 m_mMat;
        ZVector3 m_vPos;
        ZVector3 m_vCen;
        unsigned int m_lControl;
        ZVector3 m_vSize;
        float m_fRadius;
        ZEntityLocator * m_pParent;
        uint32_t m_uListID : 24;
        uint32_t m_lPotentialLightListChange : 7;
        uint32_t m_bFreezeLightList : 1;
        uint16_t m_lDrawId;
        uint16_t m_lDrawEntryId;

        uint16_t m_iPrev;
        uint16_t m_iNext;

        class ZGEOM* m_pExtraGeom;
        CQuadtreeObj* m_pDynId;
        unsigned int m_lPrim;
        const char* m_Name;

        // API
        void DoInit();
        ZGROUP* ParentGroup();
        void SetName(const char* name);
        bool SetPrim(int primId);
        void GetMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos);
        ZEntityLocator* Next();
        void SetNext(ZEntityLocator* next);
        ZEntityLocator* GetPrev();
        void SetPrev(ZEntityLocator* prev);
    }; //Size: 0x0070
    RE_VERIFY_SIZE(ZEntityLocator, 0x70);
    RE_VERIFY_OFFSET(ZEntityLocator, m_mMat, 0x0);
    RE_VERIFY_OFFSET(ZEntityLocator, m_vPos, 0x24);
    RE_VERIFY_OFFSET(ZEntityLocator, m_vCen, 0x30);
    RE_VERIFY_OFFSET(ZEntityLocator, m_lControl, 0x3C);
    RE_VERIFY_OFFSET(ZEntityLocator, m_vSize, 0x40);
    RE_VERIFY_OFFSET(ZEntityLocator, m_fRadius, 0x4C);
    RE_VERIFY_OFFSET(ZEntityLocator, m_pParent, 0x50);
    RE_VERIFY_OFFSET(ZEntityLocator, m_lDrawId, 0x58);
    RE_VERIFY_OFFSET(ZEntityLocator, m_iPrev, 0x5C);
    RE_VERIFY_OFFSET(ZEntityLocator, m_iNext, 0x5E);
    RE_VERIFY_OFFSET(ZEntityLocator, m_pExtraGeom, 0x60);
    RE_VERIFY_OFFSET(ZEntityLocator, m_pDynId, 0x64);
    RE_VERIFY_OFFSET(ZEntityLocator, m_lPrim, 0x68);
    RE_VERIFY_OFFSET(ZEntityLocator, m_Name, 0x6C);

    // Need to refactor whole codebase and use ZBaseGeom instead of ZEntityLocator
    // That name is incorrect
    using ZBaseGeom = ZEntityLocator;
}