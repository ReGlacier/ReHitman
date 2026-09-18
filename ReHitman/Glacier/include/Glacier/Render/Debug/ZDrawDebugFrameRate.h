#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Debug/ZDrawDebugObject.h>
#include <cstdint>


namespace Glacier
{
    class ZDrawDebugFrameRate : public ZDrawDebugObject
    {
    public:
        // vtbl
        void DrawFullScreen(ZDrawDebugRender* pRender) override;

        // methods
        ZDrawDebugFrameRate();

        // members
        int64_t m_lFrameCount[128];
        uint32_t m_lFrameCountPos;
    };
}
