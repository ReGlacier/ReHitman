#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SSeerObject
    {
        // methods
        bool GetVisible() const;
        void SetVisible(bool bVisible);
        int32_t GetDeltaTime() const;
        void SetDeltaTime(int32_t lTime);

        // members
        uint8_t lValue { 0 };
    };
}
