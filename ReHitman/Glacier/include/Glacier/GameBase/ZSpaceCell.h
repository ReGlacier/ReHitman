#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZSpaceCell
    {
    public:
        // static
        static const uint8_t SEEABLES_PER_CELL;
        static const uint8_t ILLEGAL_INDEX;

        // methods
        ZSpaceCell();

        uint8_t GetSeeableID(uint8_t lIndex) const;
        uint8_t GetNumSeeables() const;
        uint8_t AddSeeable(uint8_t lSeeable);
        void RemoveSeeable(uint8_t lSeeable);

        // members
        uint8_t m_iNumSeeables;
        uint8_t m_Seeables[63];
    };
    RE_VERIFY_SIZE(ZSpaceCell, 0x40);
}
