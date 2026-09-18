#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Glacier {
    struct CProjectileActivate {
        uint32_t m_rOwner;
        uint32_t m_rWeapon;
        float m_fSpeed;
        float m_fRange;
        bool m_bExplodeOnImpact;
        bool m_bCanPenetrate;
        bool m_bCanPenetrateBody;
        bool m_bSendUnderFireWarning;
        ZVector3 m_vTarget;
        float m_fPrecision;
        uint32_t m_eAmmoMaterialEnumId;
        ZItemTemplateWeapon* m_pWeaponTemplate;
    };
    RE_VERIFY_SIZE(CProjectileActivate, 0x2C);
}