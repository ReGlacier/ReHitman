#pragma once

#include <Glacier/Debug/ZDebugFloatRef.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Float debug variable that owns its value (common/zdebugii/zdebug_var.cpp).
     *
     * Unlike ZDebugFloatRef which binds to an external float, ZDebugFloat stores
     * the value internally in m_fValue and passes a reference to it to the base.
     */
    class ZDebugFloat : public ZDebugFloatRef
    {
    public:
        // methods
        ZDebugFloat(const char* pszName, const char* pszDescription,
                    float minValue, float maxValue, float step, const char* pszPath,
                    float initialValue = 0.0f);

        // members
        float m_fValue;
    };
    RE_VERIFY_SIZE(ZDebugFloat, 0x38);
}
