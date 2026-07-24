#pragma once

#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier 
{
	// fwds
	class REFTAB;
	struct CSharedCom;

	enum CCOMFormat 
	{
		CCOM_FORMAT_BOOL = 1,
		CCOM_FORMAT_CHAR = 2,
		CCOM_FORMAT_int32 = 4,
		CCOM_FORMAT_FLOAT = 8,
		CCOM_FORMAT_CLASS = 32,
	};

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

	struct CComRead
	{
		// methods
		CComRead(const CSharedCom* pCom, const char* pName);
		CComRead(const CComRead& copy);

		operator uint32_t() const;
		operator int32_t() const;
		operator float() const;
		operator char*() const;
		operator bool() const;

		// members
		const CSharedCom* m_pCom;
		const char* m_pName;
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
		// TODO: This vtable missmatches to PC, need RE more correctly!
		// vtbl
		virtual ~CSharedCom();
		virtual void Clear() = 0;
		// virtual void PrintStatus() = 0; - only in debug builds!
		virtual void SetVal(const char*, bool, CCOMType);
		virtual void SetVal(const char*, char, CCOMType);
		virtual void SetVal(const char*, uint32_t, CCOMType);
		virtual void SetVal(const char*, int32_t, CCOMType);
		virtual void SetVal(const char*, float, CCOMType);
		virtual void SetVal(const char*, bool);
		virtual void SetVal(const char*, char);
		virtual void SetVal(const char*, int);
		virtual void SetVal(const char*, float);
		virtual void SetVal(const char*, REFTAB*, CCOMType);
		virtual void SetVal(const char*, const char*, CCOMType);
		virtual void SetVal(const char*, const int*, CCOMType);
		virtual void SetVal(const char*, const char*, int, CCOMType);
		virtual void SetVal(const char*, const int*, int, CCOMType);
		virtual void SetVal(const char*, const float*, int, CCOMType);
		virtual void SetVal(char const*,char const*,int);
		virtual void SetVal(char const*,int const*,int);
		virtual void SetVal(char const*,float const*,int);
		virtual void SetVal(char const*,char const*);
		virtual void SetVal(char const*,int const*);
		virtual void SetVal(char const*,float const*);
		virtual void GetVal(char const*,bool *);
		virtual void GetVal(char const*,char *);
		virtual void GetVal(char const*,int *);
		virtual void GetVal(char const*,float *);
		virtual CComRead Get(const char*);
		virtual CComRead operator[](const char*);
		virtual int32_t GetVal(const char*, bool*&);
		virtual int32_t GetVal(const char*, char*&);
		virtual int32_t GetVal(const char*, int*&);
		virtual int32_t GetVal(const char*, float*&);
		virtual char* GetVal(const char*);
		virtual char* GetValPtr(const char*, CCOMFormat);
		virtual bool RemoveVal(const char*, int) = 0;
		virtual bool Exists(const char* , int) = 0;
		virtual int32_t GetDataLen(const char*);
		virtual void AddValOfType(const char*, int, CCOMTypeCast*, int, const char*) = 0;
		virtual ComValueInfo* GetpVal(const char*, int) = 0;
	};
	RE_VERIFY_SIZE(CSharedCom, 0x4);

	struct CCom : public CSharedCom
	{
		// vtbl
		~CCom() override;
		void Clear() override;
		bool RemoveVal(const char*, int) override;
		bool Exists(const char* , int) override;
		void AddValOfType(const char*, int, CCOMTypeCast*, int, const char*) override;
		ComValueInfo* GetpVal(const char*, int) override;
		virtual void CopyAll(CCom* pSource);

		// methods
		CCom();

		// TODO: Finish methods

		// members
		char m_Memory[16384];
		void* m_pMemory;
		REFTAB32 m_ComDat;
		ZOffsetAlloc::ZLink m_OffsetAllocLinks[16];
		ZOffsetAlloc m_OffsetAlloc;
	};
	RE_VERIFY_SIZE(CCom, 0x4148);
}