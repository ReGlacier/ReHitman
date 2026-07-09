#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <cstdint>


namespace Glacier
{
    struct SGeomEvent_Direct_Id
    {
        uint8_t m_iDirect_Value[3];
    };

    struct ZGeomEventListBuffers
    {
        struct SGeomEventListBufferEntity_Id
        {
            uint8_t m_iBuffer_Id;
            uint8_t m_iEntity_Id;
        };
    };

    struct ZGeomEventList
    {
        uint8_t m_bListEntityOffset_Or_DirectOffset;
        union 
        {
            ZGeomEventListBuffers::SGeomEventListBufferEntity_Id m_Control_Routine_List_Entity_Id;
            SGeomEvent_Direct_Id m_iControl_Routine_Direct_Id;
        };
    };
    RE_VERIFY_SIZE(ZGeomEventList, 0x4);

    struct ExGeomData
    {
        // Known bits
        // 1 - want camera msg | ZBaseGeom::WantCameraMsg
        // 2 - is moving
        int16_t _lControl;
        ZGeomEventList _Events;
        RE_ADD_PADDING(2);
        CHUNKFILE* _ExtraInitData;
    };
    RE_VERIFY_SIZE(ExGeomData, 0xC);

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
        const char* Name();
        bool DoInit();
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