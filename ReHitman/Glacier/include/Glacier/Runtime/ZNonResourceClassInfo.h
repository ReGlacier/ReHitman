#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/ZCLASSINFO.h>


namespace Glacier
{
    struct ZNonResourceClassInfo : public ZCLASSINFO
    {
        unsigned int m_lSize;
        bool m_bResourceLinked;
    };
    RE_VERIFY_SIZE(ZNonResourceClassInfo, 0x20);
}