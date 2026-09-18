#pragma once

#include <Glacier/Render/Debug/ZDrawDebugObject.h>
#include <cstdint>


namespace Glacier
{
    class ZDrawDebugTextLayer : public ZDrawDebugObject
    {
    public:
        // vtbl
        ~ZDrawDebugTextLayer() override;
        void DrawFullScreen(ZDrawDebugRender* pRender) override;
        void FrameUpdate(float fDt) override;

        // methods
        ZDrawDebugTextLayer();
        void Write(uint32_t x, uint32_t y, const char* pszText);
        void SetSize(uint32_t x, uint32_t y);
        void DrawHelp(ZDrawDebugRender* pRender);
        void DrawConsole(ZDrawDebugRender* pRender);
        static ZDrawDebugTextLayer& Instance();

        // members
        char* m_pszTextBuffer;
        uint32_t m_lSizeX;
        uint32_t m_lSizeY;
        uint32_t m_lMarginX;
        uint32_t m_lMarginY;
        uint32_t m_lTextColor;
        bool m_bDisplayHelp;
    };
}
