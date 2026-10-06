#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Items/ZItemContainer.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman
{
    class ZHM3ItemContainer : public Glacier::ZItemContainer
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemContainer, 0x100432u);

        // methods
        ZHM3ItemContainer(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // vftable
        virtual EHM3ItemType GetHM3ItemType();
        virtual bool IsDetectable();
        virtual void InitItemContainerAction() override;
        virtual bool CanContainItem(Glacier::ZItem* item) override;

        // data (total size is 0xD0, ZItemContainer size is 0xC8)
        Glacier::ZLNKOBJ* m_pGround;
        bool m_bForceUnpickable;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZHM3ItemContainer, 0xD0);
}