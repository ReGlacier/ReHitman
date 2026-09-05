#include <Glacier/Debug/sCall_u.h>
#include <cstdarg>


namespace Glacier
{
    sCall_u::sCall_u(uint32_t iLine, const char* szFile, ZDebug::uSeverityEnums eSeverity, bool bDisplayOnce, bool bIsConsoleLine)
        : m_bIsConsoleLine(bIsConsoleLine)
        , m_bDisplayOnce(bDisplayOnce)
        , m_szFile(szFile)
        , m_iLine(iLine)
        , m_eSeverity(eSeverity)
    {
    }

    void sCall_u::operator->()
    {
        // Do nothing
    }

    void sCall_u::_uPrint(const char* fmt, ...)
    {
        // TODO: Finish me
    }

    void sCall_u::_uMsg(uChannelEnums eChannel, const char* fmt, ...)
    {
        // TODO: Finish me
    }
}