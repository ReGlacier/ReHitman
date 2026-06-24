#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZEditorGroup : public ZGROUP
	{
		// Final, nothing here
	};
	RE_VERIFY_SIZE(ZEditorGroup, 0x4C); // Verified
}