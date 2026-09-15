#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GameBase/ZCellCoordinate.h>


namespace Glacier
{
    class Locator : public ZCellCoordinate
    {
    public:
        // types
        enum eInvalidLocator { INVALID = 0 };

        // static
        static const Locator Invalid;

        // methods
        Locator(const ZCellCoordinate& coord, uint8_t index);
        Locator(eInvalidLocator);
        Locator(const Locator& copy);
        Locator();

        Locator& operator=(const Locator& copy);

        bool IsValid() const;
        uint8_t GetCellIndex() const;
        void DecrementCellIndex();

        // members
        uint8_t m_Index;
    };
    RE_VERIFY_SIZE(Locator, 0x4); // PC verified
}
