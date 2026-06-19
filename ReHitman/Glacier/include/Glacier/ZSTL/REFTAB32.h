#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Glacier
{
    // Class definition REFTAB32
    class REFTAB32 : public REFTAB
    {
    public:
        char m_FirstTab[144];
    };
    RE_VERIFY_SIZE(REFTAB32, 0xAC);

    template <typename T> T* get(REFTAB32* reftab, size_t index)
    {
        return reftab ? reftab->operator[](index) : nullptr;
    }
}