#pragma once

#include <Glacier/Debug/ZDebugIntRef.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Integer debug variable that owns its value (common/zdebugii/zdebug_var.cpp).
     *
     * Unlike ZDebugIntRef which binds to an external int32_t, ZDebugInt stores
     * the value internally in m_iValue and passes a reference to it to the base.
     */
    class ZDebugInt : public ZDebugIntRef
    {
    public:
        // methods
        ZDebugInt(const char* pszName, const char* pszDescription,
                  int32_t minValue, int32_t maxValue, int32_t step, const char* pszPath);

        // members
        int32_t m_iValue;
    };
    RE_VERIFY_SIZE(ZDebugInt, 0x38);
}
