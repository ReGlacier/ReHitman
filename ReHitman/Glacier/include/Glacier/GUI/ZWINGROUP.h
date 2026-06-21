#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZWINGROUP : public ZGROUP
	{
		struct ZWINDOWS* m_pSystem;
		struct ZCONTROL* m_pFocusControl;
	}; // total size is 0x54 
	RE_VERIFY_SIZE(ZWINGROUP, 0x54);
}