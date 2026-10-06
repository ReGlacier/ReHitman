#pragma once

#include <Glacier/Items/ZItem.h>

#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman
{
    class ZHM3Item : public Glacier::ZItem
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3Item, 0x10042Au);

        // methods
        ZHM3Item(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // vftable
        virtual EHM3ItemType GetHM3ItemType();
        virtual void OverrideItemType(EHM3ItemType itemType);
        virtual void UseItemActivateAnimation();

        // methods
        void RestoreBites(uint32_t numBites);

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