#pragma once

#include <cstdint>


namespace Glacier
{
    struct IBuffer
    {
        using Callback_t = void(*)(const char*, uint32_t, void*);
        
        virtual ~IBuffer() = default;
        virtual const void* GetData(uint32_t lOffset) = 0;
    };
}