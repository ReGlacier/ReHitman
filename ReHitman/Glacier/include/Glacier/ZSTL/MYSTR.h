#pragma once

#include <Glacier/ReGlacier.h>

namespace Glacier
{
    struct MYSTR
    {
        char* String;
        char m_Buffer[124];
    };
    RE_VERIFY_SIZE(MYSTR, 0x80);
}