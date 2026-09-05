#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZAllocMany.h>
#include <cstdint>

namespace Hitman::BloodMoney
{
	class ZHM3ClipParticleControl : public Glacier::ZAllocMany
	{
	public:
		uint32_t m_iClipNr;
	};
	RE_VERIFY_SIZE(ZHM3ClipParticleControl, 0x6C); // verified
}