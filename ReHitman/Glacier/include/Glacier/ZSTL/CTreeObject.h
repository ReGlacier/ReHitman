#pragma once

#include <Glacier/ZSTL/CObjectInfo.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
	struct CTreeObject
	{
		CTreeObject* m_pNext;
		CTreeObject* m_pPrev;
		CObjectInfo m_tInfo;
	};
	RE_VERIFY_SIZE(CTreeObject, 0x18);

	struct CTreeObjectList
	{
		CTreeObject* m_pHead;
	};
	RE_VERIFY_SIZE(CTreeObjectList, 0x4);
}