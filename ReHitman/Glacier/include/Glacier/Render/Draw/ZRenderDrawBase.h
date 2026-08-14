#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/View/IView.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <cstdint>


namespace Glacier
{
    struct ZRenderViewBase
    {};
    
    class ZRenderDrawBase : public IView
    {
    public:
        // vtbl
        // methods
        // members
        bool m_bValid;
        bool m_bInitialized;
        RE_ADD_PADDING(2);
        IView* m_ViewNumberToPointers[32];
        // ZStackArrayInsert<32, ZRenderViewBase*, uint8_t> m_Views;
    };
}