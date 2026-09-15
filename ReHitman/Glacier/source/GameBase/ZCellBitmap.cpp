#include <Glacier/GameBase/ZCellCoordinate.h>
#include <Glacier/GameBase/ZCellBitmap.h>
#include <cstring>


namespace Glacier
{
    ZCellBitmap::ZCellBitmap()
    {
        memset(m_SeerCells, 0, sizeof(m_SeerCells));
    }

    ZCellBitmap::ZCellBitmap(const ZCellBitmap& copy)
    {
        memcpy(m_SeerCells, copy.m_SeerCells, sizeof(m_SeerCells));
    }

    void ZCellBitmap::Clear(ZCellCoordinate& sCoord)
    {
        uint8_t index, bit;
        GetIndexAndBit(index, bit, sCoord);

        m_SeerCells[index] &= ~(1u << bit);
    }

    void ZCellBitmap::Clear()
    {
        memset(m_SeerCells, 0, sizeof(m_SeerCells));
    }

    void ZCellBitmap::GetIndexAndBit(uint8_t& index, uint8_t& bit, const ZCellCoordinate& sCoord)
    {
        auto tmp = 0x10 * (0x10 * sCoord.m_Y + sCoord.m_Z) + sCoord.m_X;
        bit = tmp & 0x1Fu;
        index = tmp >> 5;
    }

    bool ZCellBitmap::IsSet(const ZCellCoordinate& sCoord) const
    {
        uint8_t index, bit;
        GetIndexAndBit(index, bit, sCoord);

        return ((1u << bit) & m_SeerCells[index]) != 0;
    }

    void ZCellBitmap::Set(ZCellCoordinate& sCoord)
    {
        uint8_t index, bit;
        GetIndexAndBit(index, bit, sCoord);

        m_SeerCells[index] |= (1u << bit);
    }
}
