#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/ZBitfield.h>
#include <Glacier/Items/ITEMSTATE.h>

namespace Glacier
{
    enum ITEMDEST {
        ID_ITEM = 0,
        ID_PARENT = 1,
        ID_OWNER = 2,
        ID_ROOT = 3,
        ID_FORCE32 = 2147483647,
    };

    class ZItemState : public CBaseEvent<ZGEOM>
    {
    public:
        // vftable
        virtual void SetItemGeometry(ZItemTemplate* itemTemplate, ZItem* item);
        virtual void GetItemSettings(CCom* ccom);
        virtual ZGEOM* GetUseGeom(ZItem*);
        virtual bool IsMain();
        virtual void EnableCamera(ZCAMERA*);

        // data (total size is 0x5C, ZEventBase size is 0x30)
        Glacier::ZMSGID m_msgItemStateCreate;
        Glacier::ZMSGID m_msgSetItemState;
        Glacier::ZMSGID m_msgGetItemSettings;
        ZBitfield<ITEMSTATE> m_lStateBits;
        bool m_bIsMain;
        RE_ADD_PADDING(3);
        ITEMDEST m_lDestination;
        float m_fActivateDelay;
        float m_fDeactivateTime;
        bool m_bReuseStateGeom;
        bool m_bAutoRemove;
        RE_ADD_PADDING(2);
        ZRTString m_sAnimName;
        ZREF m_rUseGeom;
        bool m_bOnlyVisible;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZItemState, 0x5C); // Verified
}