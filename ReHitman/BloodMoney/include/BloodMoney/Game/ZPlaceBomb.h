#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZRTStringObject.h>


namespace Hitman::BloodMoney
{
    enum EPlaceItemType 
    {
        PLACEITEMTYPE_BOMB = 0,
        PLACEITEMTYPE_SUITCASE = 1,
    };

    class ZPlaceBomb : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        // vtbl (not confirmed)
        virtual const char* GetActionName();
        virtual bool CanPlace();

        // data
        Glacier::REFTAB m_AffectedGeoms;
        Glacier::ZREF m_rRigGeom;
        EPlaceItemType m_iItemType;
        bool m_bPickupable;
        Glacier::ZAction* m_pAction;
        Glacier::ZItem* m_pItem;
        Glacier::Animation::Header* m_pAnim;
        Glacier::Animation::Header* m_pAnim2;
        int m_iCallBackFrame;
        Glacier::ZRTString m_szActionName;
    }; // Size 0x70
    RE_VERIFY_SIZE(ZPlaceBomb, 0x70); // Verified
}