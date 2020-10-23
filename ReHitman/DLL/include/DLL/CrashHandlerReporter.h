#pragma once

#include <cstdint>

namespace ReHitman
{
    class CrashHandlerReporter
    {
        std::intptr_t m_prevHandler;
    public:
        CrashHandlerReporter();
        ~CrashHandlerReporter();
    };
}