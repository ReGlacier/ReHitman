#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateBomb : public ZHM3ItemTemplateWeapon
    {
    public:
        // data (total size is 0x1B4, base size is 0x1A0)
        float m_fMaxDamage;
        float m_fMaxDamageRange;
        float m_fMaxRange;
        float m_fExplodeTimer;
        Glacier::ZGROUP* m_pEffectGroup;
    };
    RE_VERIFY_SIZE(ZHM3ItemTemplateBomb, 0x1B4); // Verified
}