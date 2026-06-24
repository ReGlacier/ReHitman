#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZSkipSaveGroup : public ZGROUP
	{
		// Final, nothing here
	};
	RE_VERIFY_SIZE(ZSkipSaveGroup, 0x4C); // Verified
}