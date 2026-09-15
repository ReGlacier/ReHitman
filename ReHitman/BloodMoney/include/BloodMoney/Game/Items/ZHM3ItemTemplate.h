#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplate : public Glacier::ZItemTemplate
    {
    public:
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