#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/ZNonResourceClassInfo.h>

namespace Glacier
{
	struct ZGEOMCLASSINFO : public ZNonResourceClassInfo
	{
		void(*m_pSetTypeIDAndMask)(unsigned int, unsigned int);
		unsigned int m_lType;
		unsigned int m_lGeomCases;
		const char* m_szParentClass;
		unsigned int* m_pClassId;
		unsigned int* m_pMaskId;
		uint16_t m_iClassInfoNr;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZGEOMCLASSINFO, 0x3C);
}