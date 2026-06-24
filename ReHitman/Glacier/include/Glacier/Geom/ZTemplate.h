#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZTemplate : public ZGROUP
	{
		// Final, nothing here
	};
	RE_VERIFY_SIZE(ZTemplate, 0x4C); // Verified
}