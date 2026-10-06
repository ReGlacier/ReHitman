#pragma once

#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman
{
    class ZHM3ItemTemplateAmmo : public Glacier::ZItemTemplateAmmo
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemTemplateAmmo, 0x100433u);

        // methods
        ZHM3ItemTemplateAmmo(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        //vftable
        virtual EHM3ItemType GetHM3ItemType();

        //data (total size is 0xA8, Glacier::ZItemTemplateAmmo size is 0xA4)
        EHM3ItemType m_eHM3ItemType;
    };
    RE_VERIFY_SIZE(ZHM3ItemTemplateAmmo, 0xA8); // Verified
    RE_VERIFY_OFFSET(ZHM3ItemTemplateAmmo, m_eHM3ItemType, 0xA4);
}