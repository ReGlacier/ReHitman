#pragma once

#include <Glacier/ZSTL/ZOldTypeInfo.h>


namespace Glacier
{
	struct ZBaseConRoutTypeInfo
	{
		char *m_psName; //0x0000
		uint32_t m_unk4; //0x0004
		uint32_t m_unk8; //0x0008
		uint32_t m_unkC; //0x000C
		uint32_t m_unk10; //0x0010
		uint32_t m_unk14; //0x0014
		uint32_t m_unk18; //0x0018
		uint32_t m_unk1C; //0x001C
		char *m_psClassName; //0x0020
		char *m_psParentName; //0x0024
		uint32_t m_unk28; //0x0028
		uint32_t m_unk2C; //0x002C
		uint32_t m_unk30; //0x0030
		uint32_t m_unk34; //0x0034
		uint32_t m_pProduce; //0x0038
		struct ZBaseConRoutTypeInfo *m_pNext; //0x003C
		uint32_t m_unk40; //0x0040
		uint32_t m_unk44; //0x0044
		uint32_t m_unk48; //0x0048
		uint32_t m_unk4C; //0x004C
	}; //Size: 0x0050

	class ZBaseConRout
	{
	public:
		// Internal API
		// Static
		static ZBaseConRoutTypeInfo** GetFactory();
	};
}