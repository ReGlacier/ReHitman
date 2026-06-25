#pragma once

#include <Glacier/ZSTL/ZQStaticMemBlock.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
	struct ZBaseGeomLists : public ZQStaticMemBlock
	{
	};
	RE_VERIFY_SIZE(ZBaseGeomLists, 0x18);
}