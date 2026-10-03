#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    class ZPushMemColor
    {
    public:
        ZPushMemColor(uint32_t lColor);
        ZPushMemColor(uint32_t lColor, const char* pszFile, int lLine);
        ~ZPushMemColor();

        ZPushMemColor(const ZPushMemColor& copy) = delete;
        ZPushMemColor& operator=(const ZPushMemColor& copy) = delete;

    private:
        uint32_t m_lColor{0u};
    };
    RE_VERIFY_SIZE(ZPushMemColor, 4);
}


#define PUSH_MEM_CONCAT_IMPL(a, b) a##b
#define PUSH_MEM_CONCAT(a, b) PUSH_MEM_CONCAT_IMPL(a, b)

#define PUSH_MEMORY_COLOR(color) \
    Glacier::ZPushMemColor PUSH_MEM_CONCAT(_mem_color_, __COUNTER__) { color, __FILE__, __LINE__ }