#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZSTL/REFTAB32.h>


namespace Hitman::BloodMoney
{
	class ZLoader_Sequence_Setup : public Glacier::ZSTDOBJ
	{
	public:
		Glacier::ZRTString m_Loader_Script_File;
		Glacier::REFTAB32 m_Pictures;
	};
	RE_VERIFY_SIZE(ZLoader_Sequence_Setup, 0xC0);
	RE_VERIFY_OFFSET(ZLoader_Sequence_Setup, m_Loader_Script_File, 0x10);
	RE_VERIFY_OFFSET(ZLoader_Sequence_Setup, m_Pictures, 0x14);
}