#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZEventBase.h>

namespace Glacier
{
    class ZItemState : public ZEventBase
    {
    public:
        // vftable
        virtual void SetItemGeometry(ZItemTemplate* itemTemplate, ZItem* item);
        virtual void GetItemSettings(CCom* ccom);
        virtual ZGEOM* GetUseGeom(ZItem*);
        virtual bool IsMain();
        virtual void EnableCamera(ZCAMERA*);

        // data (total size is 0x5C, ZEventBase size is 0x30)
        Glacier::ZMSGID m_MSG_ITEM_STATE_CREATE;
        Glacier::ZMSGID m_MSG_ITEM_SET_STATE;
        Glacier::ZMSGID m_MSG_ITEM_GET_ANIM_NAME;
        Glacier::ZMSGID m_field36;
        int m_field38;
        int m_isMain;
        int m_field40;
        int m_field44;
        int m_field48;
        int m_field4C;
        Glacier::ZREF m_useGeomREF;
        int m_field54;
        int m_field58;
    };
}