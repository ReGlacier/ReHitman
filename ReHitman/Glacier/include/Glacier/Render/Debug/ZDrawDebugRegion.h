#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZDrawDebugRegion
    {
    public:
        // types
        enum Align
        {
            LEFTTOP = 0,
            LEFTBOT = 1,
            RIGHTTOP = 2,
            RIGHTBOT = 3
        };

        // methods
        ZDrawDebugRegion(ZDrawDebugRegion* pParent);
        void Set(int lStartX, int lStartY, int lSizeX, int lSizeY);
        void SetSizeText(int lSizeX, int lSizeY);
        void SetPosSize(int lStartX, int lStartY, int lSizeX, int lSizeY);
        void SetPosSizeText(int lStartX, int lStartY, int lSizeX, int lSizeY);
        void DrawRect(int lPosX, int lPosY, int lSizeX, int lSizeY, uint32_t lColor);
        void Fill(uint32_t lColor);
        void DrawFocus();
        void AlignPosSizeText(ZDrawDebugRegion::Align ALign, int lStartX, int lStartY, int lSizeX, int lSizeY);

        // members
        int m_lStartX;
        int m_lStartY;
        int m_lSizeX;
        int m_lSizeY;
        int m_lBorderX;
        int m_lBorderY;
        ZDrawDebugRegion* m_pParent;
    };
}