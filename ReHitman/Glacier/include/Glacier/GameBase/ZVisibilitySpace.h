#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GameBase/ZSpaceCell.h>
#include <Glacier/GameBase/Locator.h>
#include <Glacier/Render/Debug/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZVisibilitySpace
    {
    public:
        // methods
        ZVisibilitySpace();
        void Initialize();
        ZSpaceCell& GetSection(uint32_t x, uint32_t y, uint32_t z);
        ZVector3& GetCellSize();
        Locator UpdateSeeable(uint8_t id, Locator locator, float* pos);
        void RemoveSeeable(Locator locator);
        Locator AddSeeable(const float* pos, uint8_t id);
        ZSpaceCell& GetCell(const ZCellCoordinate& coord);
        void DumpSeeables();
        void ShowSeeablePositions(ZDrawDebugRender* pRender);
        void ShowCells(ZDrawDebugRender* pRender);
        void ShowCellsForSeer(ZDrawDebugRender* pRender);
        ZCellCoordinate FindCell(const float* vPos) const;
        void GetCellPosition(float* vPos, const ZCellCoordinate& coord) const;
        bool CellIsWithinDistance(const ZCellCoordinate& coord, ZVector3& vPos, float fDistance);

        // members
        ZVector3 m_vCellSize;
        ZVector3 m_vCellSizeInv;
        ZVector3 m_vLow;
        ZVector3 m_vHigh;
        ZSpaceCell m_Sections[16][1][16];
    };
    RE_VERIFY_SIZE(ZVisibilitySpace, 0x4030);
}
