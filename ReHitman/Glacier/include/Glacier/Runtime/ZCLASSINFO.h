#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
	struct ZCLASSINFO
	{
		// methods
		ZCLASSINFO() = default;

		constexpr ZCLASSINFO(const char* psClassName, int lType)
			: m_szClassInfoName{psClassName}
			, m_iClassInfoType{lType}
			, m_lSceneInstanceCount{0}
			, Parent{nullptr}
			, Prev{nullptr}
			, Next{nullptr}
		{
		}

		int ClassInfoType() const { return m_iClassInfoType; }
		const char* ClassInfoName() const { return m_szClassInfoName; }

		// members
		const char* m_szClassInfoName{nullptr};
		int32_t m_iClassInfoType{0};
		int32_t m_lSceneInstanceCount{0};

		struct ZCLASSINFO * Parent{nullptr};
		struct ZCLASSINFO * Prev{nullptr};
		struct ZCLASSINFO * Next{nullptr};
	};
	RE_VERIFY_SIZE(ZCLASSINFO, 0x18);
}
