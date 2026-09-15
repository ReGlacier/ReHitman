#include <Glacier/GameBase/Locator.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    const Locator Locator::Invalid = Locator(Locator::INVALID);

    Locator::Locator(const ZCellCoordinate& coord, uint8_t index)
        : ZCellCoordinate(coord)
        , m_Index(index)
    {
    }

    Locator::Locator(eInvalidLocator)
        : ZCellCoordinate(ZCellCoordinate::XSEGMENTS, ZCellCoordinate::YSEGMENTS, ZCellCoordinate::ZSEGMENTS)
        , m_Index(0xFFu)
    {
    }

    Locator::Locator(const Locator& copy)
        : ZCellCoordinate(copy)
        , m_Index(copy.m_Index)
    {
    }

    Locator::Locator()
        : ZCellCoordinate()
        , m_Index(0)
    {
    }

    Locator& Locator::operator=(const Locator& copy)
    {
        m_X = copy.m_X;
        m_Y = copy.m_Y;
        m_Z = copy.m_Z;
        m_Index = copy.m_Index;
        return *this;
    }

    bool Locator::IsValid() const
    {
        return m_Index < 63 && ZCellCoordinate::IsValid();
    }

    uint8_t Locator::GetCellIndex() const
    {
        return m_Index;
    }

    void Locator::DecrementCellIndex()
    {
        ZASSERT(m_Index > 0);
        --m_Index;
    }
}
