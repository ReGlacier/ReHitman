#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Items/ZItemTemplateContainer.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman
{
    class ZHM3ItemTemplateContainer : public Glacier::ZItemTemplateContainer
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemTemplateContainer, 0x100431u);

        // methods
        ZHM3ItemTemplateContainer(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        //vftable
        virtual EHM3ItemType GetHM3ItemType();

        //data (total size is 0xB4, ZItemTemplateContainer is 0x90)
        EHM3ItemType m_eHM3ItemType;
        Glacier::ZRTString m_szHM3NormalHoldAnim;
        Glacier::ZRTString m_szHM3RunHoldAnim;
        Glacier::ZRTString m_szActorHoldAnim;
        int m_nHM3NormalHoldAnimIdx;
        int m_nHM3RunHoldAnimIdx;
        int m_nActorHoldAnimIdx;
    };
    RE_VERIFY_SIZE(ZHM3ItemTemplateContainer, 0xB4); // Verified
}