#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/CProjectileActivate.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Physics/COLI.h>


namespace Glacier
{
	struct ZProjectileBase : public CBaseEvent<ZGEOM>
	{
		// vtbl
		virtual void SetTarget(float const*);
		virtual void SetProjectileInfo(CProjectileActivate const*);
		virtual void ShotImpact(COLI*);

		// members
		bool m_bOwnerIsPlayer;
        RE_ADD_PADDING(3);
        uint32_t m_rWeaponOwner;
        uint32_t m_rWeapon;
        uint32_t m_eAmmoMaterialEnumId;
        ZItemTemplateWeapon* m_pWeaponTemplate;
        uint32_t m_lStatusFlag;
        uint16_t m_msgActivate;
        uint16_t m_msgCanPenetrate;
        uint16_t m_msgProjectileHit;
        uint16_t m_msgHitObject;
        uint16_t m_msgWarningMessage;
        RE_ADD_PADDING(2);
        COLI* m_pColi;
	};
    RE_VERIFY_SIZE(ZProjectileBase, 0x58);
}