#pragma once

#include <cstdint>

namespace Glacier::PF4
{
	enum EPathStatus : int32_t
	{
		ePathStatusOk = 0x0,
		ePathStatusBlocked = 0x1,
		ePathStatusReserved = 0x2,
	};
}