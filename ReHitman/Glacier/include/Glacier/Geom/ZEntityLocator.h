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

        struct {
            uint32_t m_uListID : 24;
            uint32_t m_lPotentialLightListChange : 7;
            uint32_t m_bFreezeLightList : 1;
        };

        union {
            uint16_t m_lDrawId;
            uint16_t m_lDrawEntryId;
        };

        union {
            uint16_t m_iRoomListNr;
            uint16_t m_iDynamicParentNr;
        };

        uint16_t m_iPrev;
        uint16_t m_iNext;

        class ZGEOM* m_pExtraGeom;
        CQuadtreeObj* m_pDynId;
        int m_lPrim;
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
    
    // Need to refactor whole codebase and use ZBaseGeom instead of ZEntityLocator
    // That name is incorrect
    using ZBaseGeom = ZEntityLocator;
}