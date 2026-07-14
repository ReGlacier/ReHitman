#pragma once

#include <Glacier/ReGlacier.h>

namespace Glacier
{
    struct ZCLASSINFO
	{
		// methods
		ZCLASSINFO(const char* psClassName, int lType)
			: m_szClassInfoName{psClassName}
			, m_iClassInfoType{lType}
		{
		}

		int ClassInfoType() const
		{
			return m_iClassInfoType;
		}

		const char* ClassInfoName() const
		{
			return m_szClassInfoName;
		}

		// members
		const char* m_szClassInfoName{nullptr};
		int m_iClassInfoType{0};
		int m_lSceneInstanceCount{0};

		struct ZCLASSINFO * Parent{nullptr};
		struct ZCLASSINFO * Prev{nullptr};
		struct ZCLASSINFO * Next{nullptr};
	};
	RE_VERIFY_SIZE(ZCLASSINFO, 0x18);
}