#include <Glacier/Debug/ZDebugIntRef.h>

#include <cstdio>
#include <cstdlib>


namespace Glacier
{
    ZDebugIntRef::ZDebugIntRef(int32_t& rValue, const char* pszName, const char* pszDescription,
                               int32_t minValue, int32_t maxValue, int32_t step, const char* pszPath)
        : ZDebugVarRef<int32_t>(rValue, pszName, pszDescription, minValue, maxValue, step, pszPath)
    {
    }

    ECLASSTYPE ZDebugIntRef::GetType()
    {
        return ECLASS_INT;
    }

    void ZDebugIntRef::Execute(uint32_t iArgC, const char** ppszArgV)
    {
        if (iArgC == 1)
        {
            // Original routes this through the debug system (sCall_u::_uPrint on PS2).
            std::printf("%s=%d\n", Name(), m_rValue);
        }
        else
        {
            m_rValue = std::atol(ppszArgV[1]);
        }
    }

    void ZDebugIntRef::CalcValue()
    {
        std::sprintf(m_szValue, "%d", m_rValue);
    }
}
