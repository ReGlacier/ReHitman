#pragma once

#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    struct SMapGroup 
    {
        Glacier::ZREF rWorldGroup;
        Glacier::ZREF rMapGroup;
        const char* szName;
        uint32_t iIndex;
    }; // Not confirmed, but should be valid
}