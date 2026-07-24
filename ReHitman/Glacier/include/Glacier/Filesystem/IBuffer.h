#pragma once

#include <cstdint>


namespace Glacier
{
    struct IBuffer
    {
        using Callback_t = void(*)(const char*, uint32_t, void*);
        
        virtual ~IBuffer() = default;
        virtual const void* GetData(uint32_t lOffset) = 0;
        virtual uint32_t GetBufferSize() = 0;
        virtual void GetBufferData(Callback_t pCallback, void* pData) = 0;
    };
}