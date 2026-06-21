#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/ZNonResourceClassInfo.h>

namespace Glacier
{
	class ZROUTCLASSINFO : public ZNonResourceClassInfo
	{
		const char* m_szRoutName;
		const char* m_szGeomName;
		int m_lPrio;
		unsigned int m_lRoutCases;
		uint16_t m_iClassInfoNr;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZROUTCLASSINFO, 0x34);
}