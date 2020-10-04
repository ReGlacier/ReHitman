#pragma once

#include <string_view>

namespace ReHitman
{
    class Logger
    {
    public:
        static void Setup();
        static void Shutdown();
        static void Assert(bool condition, std::string_view function, std::string_view file, unsigned int line, std::string_view message, std::string_view condition_str);
    };
}

#define RH_ASSERT2(condition, message) \
    do {                              \
        ReHitman::Logger::Assert((condition), __FUNCTION__, __FILE__, __LINE__, message, #condition); \
    } while (0);

#define RH_ASSERT(condition) \
    do {                     \
        ReHitman::Logger::Assert((condition), __FUNCTION__, __FILE__, __LINE__, "", #condition); \
    } while (0);