#pragma once

#include <cstdint>


namespace Glacier
{
	struct ZOldTypeInfo
	{
		char *m_psName;
		uint32_t m_unk4;
		uint32_t m_unk8;
		struct ZOldTypeInfo* m_pParentType;
		uint32_t m_unk10;
		uint32_t m_unk14;
		uint32_t m_iInnerIndex;
		uint32_t m_unk1C;
		uint32_t m_unk20;
		uint32_t m_iTypeId2;
		uint32_t m_unk28;
		char* m_psParentTypeName;
		uint32_t *m_piID;
		uint32_t *m_piMask;
		uint32_t m_unk38;
		uint32_t m_iTypeId;
		void(__cdecl* m_pProduce)(struct ZOldTypeInfo*);
		struct ZOldTypeInfo* m_pNext;
	};
}
