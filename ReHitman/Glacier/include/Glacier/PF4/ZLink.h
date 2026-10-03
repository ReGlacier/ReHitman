#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/PF4/PF4.h>
#include <cstdint>

namespace Glacier::PF4
{
	struct ZLink
	{
        int16_t m_fCost;
        ZIndex m_iNode;
        ZIndex m_Graph;
        uint8_t m_Type;
        uint8_t m_iAction;
        uint16_t m_iKeyMask;
        ZREF m_rDoorController;
	};
    RE_VERIFY_SIZE(ZLink, 0x10);
}