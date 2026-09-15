#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/CBaseEvent.h>

namespace Hitman::BloodMoney
{
	struct ZKnifeThrower : public Glacier::CBaseEvent<Glacier::ZGEOM>
	{
	public:
		Glacier::ZVector3 m_vDirection;
		Glacier::ZREF m_rOwner;
		float m_fMoveAngle;
		float m_fMoveStep;
		Glacier::ZVector3 m_vTarget;
		Glacier::ZVector3 m_vStartPos;

		/// Unlike PS2 build, PC build contains some additional members, which are used in the code, but their purpose is unknown yet
		uint8_t m_iUnk; // Looks like ZREF to smth, idk
		RE_ADD_PADDING(3);
		int* m_SomeUnknownPtr; // +0x64
	}; // size is 0x68
	RE_VERIFY_SIZE(ZKnifeThrower, 0x68);
	RE_VERIFY_OFFSET(ZKnifeThrower, m_vDirection, 0x30);
}