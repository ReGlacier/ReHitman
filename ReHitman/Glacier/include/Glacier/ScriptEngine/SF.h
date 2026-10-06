#pragma once

#include <Glacier/GlacierFWD.h>
#include <cstdint>


namespace Glacier
{
    bool SF_CheckTimeout();
    void SF_DebugPrint(const char* format, ...);
    void SF_GetZDefine(const char* pName, void* pData, uint32_t lSize);
    void SF_Input(void* pData, uint32_t lSize);
    void SF_Memcpy(void* dest, void* source, uint32_t lSize);
    void SF_Memset(void* dest, uint8_t data, uint32_t lSize);
    void SF_Pack(void* pData, uint32_t lSize);
    void SF_Unpack(void* pData, uint32_t lSize);
}