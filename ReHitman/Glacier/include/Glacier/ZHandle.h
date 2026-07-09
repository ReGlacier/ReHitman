#pragma once

#include <cstdint>
#include <Glacier/ReGlacier.h>


namespace Glacier
{
    class ZActionMapTree;

    struct ZHandle
    {
        const char* m_szName;
        ZActionMapTree* m_pkMap;
        int32_t m_iSeq;
    };
    RE_VERIFY_SIZE(ZHandle, 0xC);
}