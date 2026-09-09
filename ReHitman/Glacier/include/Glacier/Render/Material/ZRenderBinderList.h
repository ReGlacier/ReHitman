#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderBinderList
    {
    public:
        // vtbl
        virtual ~ZRenderBinderList();

        // methods
        ZRenderBinderList() = default;
        void ExecuteBinders(const ZRenderContext* pContext) const;

        // members
        uint32_t m_lNumBinders { 0u };
        ZRenderBinder** m_pBinders { nullptr };
    };
    RE_VERIFY_SIZE(ZRenderBinderList, 0xC); // Approved PC
}