#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZGlobalComponentBase.h>
#include <cstdint>


namespace Glacier
{
    class ZSysmemDebugHandler : public ZGlobalComponentBase
    {
    public:
        // vtbl
        ~ZSysmemDebugHandler() override = default;
        virtual void PreAllocChange(uint32_t&) = 0;
        virtual void PostAllocChange(char*&, uint32_t&) = 0;
        virtual void PreFreeChange(char*&) = 0;
        virtual void PostAlloc(char* pStart, uint32_t iSize, char* pRealStart, char* pRealEnd) = 0;
        virtual void PreFree(char* pStart) = 0;
        virtual void PreShrinkChange(char*& pStart, uint32_t& lNewSize) = 0;
        virtual void Reset() = 0;
    };
}