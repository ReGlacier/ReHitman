#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/CProjectileActivate.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
	class ZProjectileBase : public CBaseEvent<ZGEOM>
	{
	public:
		// static
		// Reversed from PC 0x53F130 (engine\eventsbase\zprojectile.cpp). Initialize() registers the
		// Activate/CanPenetrate/ProjectileHit/HitObject/Explode_Warning messages, allocates the COLI
		// and caches the three scene refs below.
		STATIC_CLASS_VAR(ZProjectileBase, uint32_t, m_rBulletMarks);
		STATIC_CLASS_VAR(ZProjectileBase, uint32_t, m_rCrowd);
		STATIC_CLASS_VAR(ZProjectileBase, uint32_t, m_rBloodSplatters);

		// PS2 0x2E22A0 / iOS 0x10031BC0C: material-property ids resolved once through
		// BS_Runtime::ZMaterialDescriptionDB::GetPropertyId.
		STATIC_CLASS_VAR(ZProjectileBase, int32_t, m_MaterialProperty_SoundEnter);
		STATIC_CLASS_VAR(ZProjectileBase, int32_t, m_MaterialProperty_DebrisEnter);
		STATIC_CLASS_VAR(ZProjectileBase, int32_t, m_MaterialProperty_DebrisExit);
		STATIC_CLASS_VAR(ZProjectileBase, int32_t, m_MaterialProperty_BulletHole);
		STATIC_CLASS_VAR(ZProjectileBase, int32_t, m_MaterialProperty_BulletHoleSize);
		STATIC_CLASS_VAR(ZProjectileBase, int32_t, m_MaterialProperty_BulletHoleSizeVariation);
		STATIC_CLASS_VAR(ZProjectileBase, int32_t, m_MaterialProperty_BloodSplatter);

		// RTTI (rout event class: DECLARE_ROUT_CLASS declares the class info, the factory
		// producer and the RTP Info; both are defined in ZProjectileBase.cpp).
		DECLARE_ROUT_CLASS(ZProjectileBase, ZGEOM, ProjectileBase, 48, 0);

		// vtbl
		~ZProjectileBase() override;
		// RTP::cBase
		const RTP::ZPropertyInfo& GetProperties() const override;
		// ZProjectileBase
		virtual void SetTarget(float const*);
		virtual void SetProjectileInfo(CProjectileActivate const*);
		virtual void ShotImpact(COLI*);

		// methods
		// PC 0x540760 (inlined into ZProjectile::Ctor) / PS2 0x2E222C.
		ZProjectileBase();
		// PC 0x53F130 / PS2 0x2E22A0 / iOS 0x10031BC0C.
		void Initialize();

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