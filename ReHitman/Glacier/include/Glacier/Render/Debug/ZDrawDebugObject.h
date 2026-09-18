#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Debug/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZDrawDebugObject
    {
    public:
        // vtbl
        virtual ~ZDrawDebugObject();
        virtual void Draw(ZDrawDebugRender* pRender);
        virtual void DrawFullScreen(ZDrawDebugRender* pRender);
        virtual void FrameUpdate(float fDt);

        // methods
        ZDrawDebugObject();
        void SetColot(uint32_t lColor);

        // members
        uint32_t m_lColor;
    };
}