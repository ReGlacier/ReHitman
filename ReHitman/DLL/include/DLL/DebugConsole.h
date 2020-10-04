#pragma once

#include <string_view>

namespace ReHitman
{
    class DebugConsole
    {
        static bool g_bIsCreated;
    public:
        static void Create(std::string_view caption);
    };
}