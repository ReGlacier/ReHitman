#include <Glacier/Debug/ZDebugFloat.h>


namespace Glacier
{
    ZDebugFloat::ZDebugFloat(const char* pszName, const char* pszDescription,
                             float minValue, float maxValue, float step, const char* pszPath,
                             float initialValue)
        : ZDebugFloatRef(m_fValue, pszName, pszDescription, minValue, maxValue, step, pszPath)
        , m_fValue(initialValue)
    {
    }
}
