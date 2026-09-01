#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/PF4.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::PF4
{
	struct ZPath
	{
		void GetPosition(int iPathEntryIndex, ZVector3& vPosition) const;
		bool GetNormal(int iPathEntryIndex, ZVector3& vNormal) const;
		float GetDistanceFromStart(int iPathEntryIndex) const;

		ZDataRef* m_pathIdx;
		int m_iMaxSize;
		ZVector3 m_Vertices[4];
		int m_CustomVertices;
		ZInterface* m_PathFinder;
		int m_Size;
		float m_Cost;
	};
	RE_VERIFY_SIZE(ZPath, 0x48);
}
