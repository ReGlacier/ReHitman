#pragma once

#include <Glacier/ReGlacier.h>

namespace Glacier
{
    struct ZCLASSINFO
	{
		const char* m_szClassInfoName;
		int m_iClassInfoType;
		int m_lSceneInstanceCount;

		struct ZCLASSINFO * Parent;
		struct ZCLASSINFO * Prev;
		struct ZCLASSINFO * Next;
	};
	RE_VERIFY_SIZE(ZCLASSINFO, 0x18);
}