#pragma once

#include <Glacier/PF4/ZLocation.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>

namespace Glacier::PF4
{
	struct ZPathFollower;

	struct ZMetaNode
	{
		void* m_Data;
		int m_Type;
		ZPathFollower* m_Follower;
		ZMetaNode* m_Next;
		ZMetaNode* m_Prev;
		ZLocation m_Location;
	};
	RE_VERIFY_SIZE(ZMetaNode, 0x28);
}