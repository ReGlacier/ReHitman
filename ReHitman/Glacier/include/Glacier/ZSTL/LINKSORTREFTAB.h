#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/LINKREFTAB.h>

namespace Glacier
{
    class LINKSORTREFTAB : public LINKREFTAB
    {
    public:
        /// === vftable ===
        /// === members ===
        unsigned int *m_pLastAddSort;
    };
    RE_VERIFY_SIZE(LINKSORTREFTAB, 0x2C);
}