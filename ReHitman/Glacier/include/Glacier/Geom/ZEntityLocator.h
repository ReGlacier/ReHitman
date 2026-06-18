#pragma once

#include <G1ConfigurationService.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>

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
        int m_iEntryIndex;
        class ZGEOM* m_pExtraGeom;
        int m_pDynId;
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

    static_assert(sizeof(ZEntityLocator) == 0x70, "Bad size of ZEntityLocator (ZBaseGeom)");

    // Need to refactor whole codebase and use ZBaseGeom instead of ZEntityLocator
    // That name is incorrect
    using ZBaseGeom = ZEntityLocator;
}