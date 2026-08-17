#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Debug/ZDrawDebugRegion.h>
#include <cstdint>


namespace Glacier
{
    class ZDebugFrame : public ZDrawDebugRegion
    {
    public:
        // vtbl
        virtual ZDebugFrame* AddFrame();

        // methods
        ZDebugFrame(ZDebugFrame* pParent);
        void SetTextColor(uint32_t lColor);
        void DrawLine(float fStartX, float fStartY, float fEndX, float fEndY, uint32_t lColor);
        int Plot(uint32_t lLinePosX, uint32_t lLinePosY, const char* Format, ...);
        int PlotInvers(uint32_t lLinePosX, uint32_t lLinePosY, const char* Format, ...);
        int PlotFixed(uint32_t lLinePosX, uint32_t lLinePosY, const char* pText, const char* Format, ...);
        int PlotFixedInvers(uint32_t lLinePosX, uint32_t lLinePosY, const char* pText, const char* Format, ...);

        // members
        uint32_t m_dwTextColor;
        uint32_t m_dwTextColorInv;
        uint32_t m_dwBgColorInv;
    };
}