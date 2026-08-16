#include <Glacier/Debug/ZPushMemColor.h>
#include <Glacier/System/ZSysMem.h>


namespace Glacier
{
    ZPushMemColor::ZPushMemColor(uint32_t lColor)
    {
        m_lColor = SetMemColor(lColor);
    }

    ZPushMemColor::ZPushMemColor(uint32_t lColor, const char* pszFile, int lLine)
    {
        m_lColor = SetMemColor(lColor);
        ZSysMem::Instance().SetFileLine(pszFile, lLine);
    }

    ZPushMemColor::~ZPushMemColor()
    {
        SetMemColor(m_lColor);
    }
}