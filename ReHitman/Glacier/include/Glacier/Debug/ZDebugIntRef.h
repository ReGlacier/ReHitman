#pragma once

#include <Glacier/Debug/ZDebugVarRef.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Integer debug variable bound to an external int32_t
     * (common/zdebugii/zdebug_var.cpp).
     *
     * Execute() with no arguments prints "name=value" through the debug output,
     * with one argument parses it with atol() and assigns the referenced value.
     */
    class ZDebugIntRef : public ZDebugVarRef<int32_t>
    {
    public:
        // methods
        ZDebugIntRef(int32_t& rValue, const char* pszName, const char* pszDescription,
                     int32_t minValue, int32_t maxValue, int32_t step, const char* pszPath);

        ECLASSTYPE GetType() override;
        void Execute(uint32_t iArgC, const char** ppszArgV) override;
        void CalcValue() override;
    };
    RE_VERIFY_SIZE(ZDebugIntRef, 0x34);
}
