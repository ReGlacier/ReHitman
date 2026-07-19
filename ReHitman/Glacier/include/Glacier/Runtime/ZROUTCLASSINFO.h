#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/ZNonResourceClassInfo.h>

namespace Glacier
{
	struct ZROUTCLASSINFO : public ZNonResourceClassInfo
	{
		// methods
		ZROUTCLASSINFO(
			const char* psClassName,
        	uint32_t lClassSize,
        	const char* psUnused,
        	const char* psEventName,
        	const char* psHelpText,
        	int lRoutType,
        	uint32_t lFlags,
        	const char* psBaseClassName);
		ZROUTCLASSINFO(const ZROUTCLASSINFO& copy);
		~ZROUTCLASSINFO();

		uint32_t RoutCases() const;
		const char* RoutName() const;

		// members
		const char* m_szRoutName;
		const char* m_szGeomName;
		int m_lPrio;
		uint32_t m_lRoutCases;
		uint16_t m_iClassInfoNr;
		RE_ADD_PADDING(2);
	};
	RE_VERIFY_SIZE(ZROUTCLASSINFO, 0x44);
}