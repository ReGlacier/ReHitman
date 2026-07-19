#include <Glacier/Runtime/ZROUTCLASSINFO.h>


namespace Glacier
{
    ZROUTCLASSINFO::ZROUTCLASSINFO(
        const char* psClassName,
        uint32_t lClassSize,
        const char* psUnused,
        const char* psEventName,
        const char* psHelpText,
        int lRoutType,
        uint32_t lFlags,
        const char* psBaseClassName)
        : ZNonResourceClassInfo(psClassName, 1, lClassSize, psBaseClassName)
        , m_szRoutName(psEventName)
        , m_szGeomName(psHelpText)
        , m_lPrio(lRoutType)
        , m_lRoutCases(lFlags)
    {
    }

    ZROUTCLASSINFO::ZROUTCLASSINFO(const ZROUTCLASSINFO& copy) = default;
	ZROUTCLASSINFO::~ZROUTCLASSINFO() = default;
		
    uint32_t ZROUTCLASSINFO::RoutCases() const
    { 
        return m_lRoutCases; 
    }

    const char* ZROUTCLASSINFO::RoutName() const 
    { 
        return m_szRoutName; 
    }
}