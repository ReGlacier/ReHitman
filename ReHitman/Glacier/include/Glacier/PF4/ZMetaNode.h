#pragma once

#include <Glacier/PF4/ZLocation.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    class ZBoid;
}

namespace Glacier::PF4
{
	struct ZPathFollower;

	struct ZMetaNode
	{
		// methods
		ZMetaNode();
		ZMetaNode& operator=(const ZMetaNode& other);
		const ZVector3& GetPosition() const;
		const ZLocation& GetLocation() const;
		bool IsRegistered() const;

		// members
		void* m_Data{nullptr};
		int m_Type{0};
		ZBoid* m_pBoid{nullptr};
		ZMetaNode* m_Next{nullptr};
		ZMetaNode* m_Prev{nullptr};
		ZLocation m_Location{};
	};
	RE_VERIFY_SIZE(ZMetaNode, 0x28);
}
