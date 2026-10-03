#pragma once

#include <Glacier/Debug/ZDebugVarRef.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Float debug variable bound to an external float
     * (common/zdebugii/zdebug_var.cpp).
     *
     * Execute() with no arguments prints "name=value" through the debug output,
     * with one argument parses it with atof() and assigns the referenced value.
     */
    class ZDebugFloatRef : public ZDebugVarRef<float>
    {
    public:
        // methods
        ZDebugFloatRef(float& rValue, const char* pszName, const char* pszDescription,
                       float minValue, float maxValue, float step, const char* pszPath);

        ECLASSTYPE GetType() override;
        void Execute(uint32_t iArgC, const char** ppszArgV) override;
        void CalcValue() override;
    };
    RE_VERIFY_SIZE(ZDebugFloatRef, 0x34);
}
