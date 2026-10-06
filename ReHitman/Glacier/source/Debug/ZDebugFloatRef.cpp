#include <Glacier/Debug/ZDebugFloatRef.h>

#include <cstdio>
#include <cstdlib>


namespace Glacier
{
    ZDebugFloatRef::ZDebugFloatRef(float& rValue, const char* pszName, const char* pszDescription,
                                   float minValue, float maxValue, float step, const char* pszPath)
        : ZDebugVarRef<float>(rValue, pszName, pszDescription, minValue, maxValue, step, pszPath)
    {
    }

    ECLASSTYPE ZDebugFloatRef::GetType()
    {
        return ECLASS_FLOAT;
    }

    void ZDebugFloatRef::Execute(uint32_t iArgC, const char** ppszArgV)
    {
        if (iArgC == 1)
        {
            // Original routes this through the debug system (sCall_u::_uPrint on PS2/XBOX).
            std::printf("%s=%f\n", Name(), static_cast<double>(m_rValue));
        }
        else
        {
            m_rValue = static_cast<float>(std::atof(ppszArgV[1]));
        }
    }

    void ZDebugFloatRef::CalcValue()
    {
        std::sprintf(m_szValue, "%f", static_cast<double>(m_rValue));
    }
}
