#pragma once

#include <Glacier/Debug/ZDebugCommand.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Debug variable bound to an external value (common/zdebugii/zdebug_var.cpp).
     *
     * Inc()/Dec() step the referenced value clamped to [m_MinValue, m_MaxValue].
     * Execute() without arguments prints the current value, with an argument assigns it.
     *
     * @tparam T Arithmetic value type (int32_t or float in the original game).
     */
    template <typename T>
    class ZDebugVarRef : public ZDebugCommand
    {
    public:
        // methods
        ZDebugVarRef(T& rValue, const char* pszName, const char* pszDescription,
                     T minValue, T maxValue, T step, const char* pszPath)
            : ZDebugCommand(pszName, pszDescription)
            , m_rValue(rValue)
            , m_MinValue(minValue)
            , m_MaxValue(maxValue)
            , m_Step(step)
        {
            m_pszPath = pszPath;
        }

        void Inc() override
        {
            if (m_rValue + m_Step > m_MaxValue)
                m_rValue = m_MaxValue;
            else
                m_rValue = m_rValue + m_Step;
        }

        void Dec() override
        {
            if (m_rValue - m_Step < m_MinValue)
                m_rValue = m_MinValue;
            else
                m_rValue = m_rValue - m_Step;
        }

        operator T() const
        {
            return m_rValue;
        }

        ZDebugVarRef& operator=(const T& rhs)
        {
            m_rValue = rhs;
            return *this;
        }

        // members
        T& m_rValue;
        T m_MinValue;
        T m_MaxValue;
        T m_Step;
    };
    RE_VERIFY_SIZE(ZDebugVarRef<int32_t>, 0x34);
    RE_VERIFY_SIZE(ZDebugVarRef<float>, 0x34);
}
