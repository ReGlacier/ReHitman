#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderBinder
    {
    public:
        // vtbl
        virtual ~ZRenderBinder();
        virtual void Execute(const ZRenderContext* pContext) = 0;
        virtual void PrintInfo();
        virtual bool IsParameterUsed(const char* pszParameter);

        // methods
        ZRenderBinder() = default;

        // members
        const char* m_pszName { nullptr }; //+0x4
        uint32_t m_lBinderType { 0u }; //+0x8
    };
    RE_VERIFY_SIZE(ZRenderBinder, 0xC); // Verified PC
}