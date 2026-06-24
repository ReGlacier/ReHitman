#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZPATHGROUP : public ZGROUP
	{
		REFTAB* m_prtLines;
	};
	RE_VERIFY_SIZE(ZPATHGROUP, 0x50); // Verified
}