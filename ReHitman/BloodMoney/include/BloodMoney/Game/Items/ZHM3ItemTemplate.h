#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman
{
    class ZHM3ItemTemplate : public Glacier::ZItemTemplate
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemTemplate, 0x100428u);

        // methods
        ZHM3ItemTemplate(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // vftable
        virtual EHM3ItemType GetHM3ItemType();

        // data (total size is 0x94, ZItemTemplate size is 0x74)
        EHM3ItemType m_eHM3ItemType;
        Glacier::ZRTString m_szHM3NormalHoldAnim;
        Glacier::ZRTString m_szHM3RunHoldAnim;
        Glacier::ZRTString m_szActorHoldAnim;
        int m_nHM3NormalHoldAnimIdx;
        int m_nHM3RunHoldAnimIdx;
        int m_nActorHoldAnimIdx;
        uint8_t m_iNumBites;
        bool m_bDrinkable;
        bool m_bEdible;
        RE_ADD_PADDING(1);
    };
    RE_VERIFY_SIZE(ZHM3ItemTemplate, 0x94); // Verified
}