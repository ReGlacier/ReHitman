#include <Glacier/GameBase/ZCellCoordinate.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    const int8_t ZCellCoordinate::XSEGMENTS = 0x10;
    const int8_t ZCellCoordinate::YSEGMENTS = 0x01;
    const int8_t ZCellCoordinate::ZSEGMENTS = 0x10;

    ZCellCoordinate::ZCellCoordinate(int8_t x, int8_t y, int8_t z)
        : m_X(x), m_Y(y), m_Z(z)
    {
    }

    ZCellCoordinate::ZCellCoordinate()
        : m_X(), m_Y(0), m_Z(0)
    {
    }

    bool ZCellCoordinate::IsValid() const
    {
        return static_cast<uint8_t>(m_X) < static_cast<uint8_t>(XSEGMENTS)
            && static_cast<uint8_t>(m_Y) < static_cast<uint8_t>(YSEGMENTS)
            && static_cast<uint8_t>(m_Z) < static_cast<uint8_t>(ZSEGMENTS);
    }

    bool ZCellCoordinate::operator==(const ZCellCoordinate& rhs) const
    {
        return m_X == rhs.m_X && m_Y == rhs.m_Y && m_Z == rhs.m_Z;
    }

    ZCellCoordinate ZCellCoordinate::Shift(int8_t dx, int8_t dy, int8_t dz) const
    {
        int8_t nx = clamp<int8_t>(m_X + dx, 0, XSEGMENTS - 1);
        int8_t ny = clamp<int8_t>(m_Y + dy, 0, YSEGMENTS - 1);
        int8_t nz = clamp<int8_t>(m_Z + dz, 0, ZSEGMENTS - 1);

        return ZCellCoordinate { nx, ny, nz };
    }

    bool ZCellCoordinate::IsWithin(const ZCellCoordinate& a, const ZCellCoordinate& b) const
    {
        return m_X >= a.m_X && m_Y >= a.m_Y && m_Z >= a.m_Z && b.m_X >= m_X && b.m_Y >= m_Y && b.m_Z >= m_Z;
    }
}
