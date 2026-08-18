#include <Glacier/Debug/ZDebugInt.h>


namespace Glacier
{
    ZDebugInt::ZDebugInt(const char* pszName, const char* pszDescription,
                         int32_t minValue, int32_t maxValue, int32_t step, const char* pszPath)
        : ZDebugIntRef(m_iValue, pszName, pszDescription, minValue, maxValue, step, pszPath)
        , m_iValue(0)
    {
    }
}
