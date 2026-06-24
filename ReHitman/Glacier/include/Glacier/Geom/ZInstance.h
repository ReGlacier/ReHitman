#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	struct ZInstance : public ZGROUP
	{
		unsigned int m_rTemplate;
	};
	RE_VERIFY_SIZE(ZInstance, 0x50); // Verified
}