#include <BloodMoney/Engine/ZUTC4.h>
#include <cstdint>

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        constexpr std::intptr_t kGetNextUTC4CharFuncAddr = 0x004390C0;
    }

    int ZUTC4::GetNextUTC4Char(const char*& arg)
    {
        typedef int(__cdecl* GetNextUTC4Char_t)(const char*&);
        auto func = (GetNextUTC4Char_t)Consts::kGetNextUTC4CharFuncAddr;
        return func(arg);
    }
}