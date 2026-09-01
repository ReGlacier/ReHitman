#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZCellCoordinate
    {
    public:
        // static
        static const int8_t XSEGMENTS;
        static const int8_t YSEGMENTS;
        static const int8_t ZSEGMENTS;

        // methods
        ZCellCoordinate(int8_t x, int8_t y, int8_t z);
        ZCellCoordinate();
        bool IsValid() const;
        bool operator==(const ZCellCoordinate& rhs) const;
        ZCellCoordinate Shift(int8_t dx, int8_t dy, int8_t dz) const;
        bool IsWithin(const ZCellCoordinate& a, const ZCellCoordinate& b) const;

        // members
        int8_t m_X;
        int8_t m_Y;
        int8_t m_Z;
    };
    RE_VERIFY_SIZE(ZCellCoordinate, 0x3); // PC verified
}
