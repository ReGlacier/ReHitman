#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>

#include <Glacier/PF4/PF4.h>


namespace Glacier::PF4
{
    struct ZDataRef
    {
        uint32_t m_Id : 14;
        uint32_t m_Type : 2;
        ZVector2 m_Pos;
    };
    RE_VERIFY_SIZE(ZDataRef, 0xC);
}