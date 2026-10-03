#include <Glacier/Runtime/ZROUTCLASSINFO.h>


namespace Glacier
{
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
