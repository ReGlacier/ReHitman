#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Items/ZItemTemplateContainer.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateContainer : public Glacier::ZItemTemplateContainer
    {
    public:
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