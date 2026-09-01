#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZCellCoordinate;

    class ZCellBitmap
    {
    public:
        // methods
        ZCellBitmap();
        ZCellBitmap(const ZCellBitmap& copy);

        void Clear(ZCellCoordinate& sCoord);
        void Clear();
        static void GetIndexAndBit(uint8_t& index, uint8_t& bit, const ZCellCoordinate& sCoord);
        bool IsSet(const ZCellCoordinate& sCoord) const;
        void Set(ZCellCoordinate& sCoord);

        // members
        uint32_t m_SeerCells[8];
    };
    RE_VERIFY_SIZE(ZCellBitmap, 0x20); // PC verified
}
