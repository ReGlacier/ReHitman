#include <DLL/Logger.h>
#include <spdlog/spdlog.h>

#include <Windows.h>

namespace ReHitman
{
    void Logger::Setup()
    {
    }

    void Logger::Shutdown()
    {
        spdlog::shutdown();
    }

    void Logger::Assert(bool condition,
                        std::string_view function,
                        std::string_view file,
                        unsigned int line,
                        std::string_view message,
                        std::string_view condition_str)
    {
        if (!condition)
        {
            const auto msg = fmt::format(
                    "ASSERTION FAILED!\n\n"
                    "Condition: {}\n"
                    "Function: {}\n"
                    "Line: {}\n"
                    "File: {}\n",
                    condition_str.data(),
                    function.data(),
                    line,
                    file.data()
            );

            MessageBox(nullptr, msg.data(), "ASSERTION FAILED!", MB_ICONEXCLAMATION | MB_OK);
            ExitProcess(EXIT_FAILURE);
        }
    }
}