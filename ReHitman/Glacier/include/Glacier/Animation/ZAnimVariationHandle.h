#pragma once

#include <cstdint>

namespace Glacier
{
    struct ZAnimVariationHandle
    {
        // methods
        ZAnimVariationHandle();
        ZAnimVariationHandle(const ZAnimVariationHandle& copy);
        ZAnimVariationHandle& operator=(const ZAnimVariationHandle& copy);

        bool IsValid() const;

        // members
        int16_t iIndex { -1 };
    };
}