#pragma once

#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ReGlacier.h>


namespace Glacier 
{
	enum CCOMType : uint32_t
	{
		CCOM_TYPE_INVALID = 0xFFFFFFFF,
		CCOM_TYPE_BOOL = 0x0,
		CCOM_TYPE_CHAR = 0x1,
		CCOM_TYPE_INT32 = 0x2,
		CCOM_TYPE_FLOAT = 0x3,
		CCOM_TYPE_REF = 0x4,
		CCOM_TYPE_PTR = 0x5,
		CCOM_TYPE_V3 = 0x6,
		CCOM_TYPE_FV3 = 0x7,
		CCOM_TYPE_DV3 = 0x8,
		CCOM_TYPE_M33 = 0x9,
		CCOM_TYPE_FM33 = 0xA,
		CCOM_TYPE_DM33 = 0xB,
		CCOM_TYPE_STRING = 0xC,
		CCOM_TYPE_BLOCK = 0xD,
		CCOM_TYPE_FILE = 0xE,
		CCOM_TYPE_DATA = 0xF,
		CCOM_TYPE_GEOMREF = 0x10,
		CCOM_TYPE_GEOMREFTAB = 0x11,
		CCOM_TYPE_ZMSG = 0x12,
		CCOM_TYPE_AUDIOREF = 0x13,
	};

	struct CCOMTypeCast
	{
		union {
			int lLetters;
			bool bLetters[4];
		};

		int lTypeLen;
		CCOMType eComType;
		int eLEn;
		int eCComFormat;
	};

	struct ComValueInfo
	{
		int lNameLen;
		CCOMTypeCast* lType;
		int lDataLen;
	};

	struct CGlobalCom
	{
		// Types
		struct CGlobalComMemInfo
		{
			uint32_t size;
			bool used;
		};
		RE_VERIFY_SIZE(CGlobalComMemInfo, 0x8);

		// Vtbl
		virtual ~CGlobalCom();
		virtual void Clear();
		virtual bool RemoveVal(const char*, int);
		virtual bool Exists(const char*, int);
		virtual void AddValueOfType(const char*, int, CCOMTypeCast*, int, const char*);
		virtual ComValueInfo* GetpVal(const char*, int);
		virtual bool EnumKeys(char*, int, unsigned int);

		// Members
		int maxSize;
		void* memory;
		int memoryWritePtr;
	};
	RE_VERIFY_SIZE(CGlobalCom, 0x10);

	struct CSharedCom
	{
		virtual ~CSharedCom();

		// TODO: Finish me later pls
	};
	RE_VERIFY_SIZE(CSharedCom, 0x4);

	struct CCom : public CSharedCom
	{
		// members
		char m_Memory[16384];
		void* m_pMemory;
		REFTAB32 m_ComDat;
		ZOffsetAlloc::ZLink m_OffsetAllocLinks[16];
		ZOffsetAlloc m_OffsetAlloc;
	};
	RE_VERIFY_SIZE(CCom, 0x4148);
}