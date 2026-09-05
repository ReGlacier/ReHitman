#pragma once

#include <Glacier/ZSTL/CObjectInfo.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
	struct CTreeObject
	{
		// methods
		CTreeObject(int iID)
			: m_pNext{nullptr}
			, m_pPrev{nullptr}
			, m_tInfo{}
		{
			m_tInfo.iID = iID;
		}
		
		// members
		CTreeObject* m_pNext;
		CTreeObject* m_pPrev;
		CObjectInfo m_tInfo;
	};
	RE_VERIFY_SIZE(CTreeObject, 0x18);

	struct CTreeObjectList
	{
		CTreeObject* m_pHead;

		void Attach(CTreeObject* pObject);
		void Detach(CTreeObject* pObject);
		CTreeObject* GetHead() const;
	};
	RE_VERIFY_SIZE(CTreeObjectList, 0x4);
}