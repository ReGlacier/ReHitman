#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    struct SHitInfo
    {
        struct SUnknown;

        SUnknown* m_unk0;
        int m_field4;
        int m_ref8;
        int m_refC;
    };

    struct SHitInfo::SUnknown {
        int m_field0;
        int m_field4;
        int m_field8;
        ZVector3 m_vec0;
        ZVector3 m_vec4;
    };
}