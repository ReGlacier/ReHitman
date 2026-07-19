#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZSkipSaveGroup : public ZGROUP
	{
		// Final, nothing here
		STATIC_CLASS_VAR(ZSkipSaveGroup, uint32_t, m_Id);
        STATIC_CLASS_VAR(ZSkipSaveGroup, uint32_t, m_Mask);
	};
	RE_VERIFY_SIZE(ZSkipSaveGroup, 0x4C); // Verified
}