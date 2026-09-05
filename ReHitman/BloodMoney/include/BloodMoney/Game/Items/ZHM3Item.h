#pragma once

#include <Glacier/Items/ZItem.h>

#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3Item : public Glacier::ZItem
    {
    public:
        // vftable
        virtual EHM3ItemType GetHM3ItemType();
        virtual void OverrideItemType(EHM3ItemType itemType);
        virtual void UseItemActivateAnimation();

        // data (total size is 0x9C, ZItem size is 0x84)
        Glacier::ZLNKOBJ* m_pGround;
        Glacier::Animation::Header* m_pAnimUse;
        EHM3ItemType m_eOverriddenType;
        uint32_t m_ItemProperties;
        uint32_t m_iNumBitesRemoved;
        bool m_bForceUnpickable;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZHM3Item, 0x9C); // Verified
}