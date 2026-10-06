#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZAllocMany.h>
#include <cstdint>

namespace Hitman
{
	class ZHM3ItemWeapon;

	class ZHM3ClipParticleControl : public Glacier::ZAllocMany
	{
	public:
		// PC 0x64CED0. Pops one casing/shell out of the pool, drops it on the given bone matrix and
		// position, and launches it with the pos + speed*scale rigid-body velocity (1.3 for a
		// six-shooter, 5.0 otherwise). Does nothing when the weapon's owner is an actor holding it
		// far from the player.
		void SpawnClipAtMatPosSpeed(const Glacier::ZMat3x3& mat, const Glacier::ZVector3& pos,
			const Glacier::ZVector3& speed, ZHM3ItemWeapon* pWeapon, bool bSixShooter);

		uint32_t m_iClipNr;
	};
	RE_VERIFY_SIZE(ZHM3ClipParticleControl, 0x6C); // verified
}
