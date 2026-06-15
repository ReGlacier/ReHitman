#pragma once

#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>


namespace Glacier
{
	enum CCOMType : int {
		CCOM_TYPE_INVALID = -1,
		CCOM_TYPE_BOOL = 0,
		CCOM_TYPE_CHAR = 1,
		CCOM_TYPE_INT32 = 2,
		CCOM_TYPE_FLOAT = 3,
		CCOM_TYPE_REF = 4,
		CCOM_TYPE_PTR = 5,
		CCOM_TYPE_V3 = 6,  // Vector3
		CCOM_TYPE_FV3 = 7,  // Float Vector3
		CCOM_TYPE_DV3 = 8,  // Double Vector3
		CCOM_TYPE_M33 = 9,  // Matrix 3x3
		CCOM_TYPE_FM33 = 10, // Float Matrix 3x3
		CCOM_TYPE_DM33 = 11, // Double Matrix 3x3
		CCOM_TYPE_STRING = 12,
		CCOM_TYPE_BLOCK = 13,
		CCOM_TYPE_FILE = 14,
		CCOM_TYPE_DATA = 15,
		CCOM_TYPE_GEOMREF = 16,
		CCOM_TYPE_GEOMREFTAB = 17,
		CCOM_TYPE_ZMSG = 18,
		CCOM_TYPE_AUDIOREF = 19
	};

	enum CCOMFormat : int {
		CCOM_FORMAT_BOOL = 1,
		CCOM_FORMAT_CHAR = 2,
		CCOM_FORMAT_int32 = 4,
		CCOM_FORMAT_FLOAT = 8,
		CCOM_FORMAT_CLASS = 32
	};

	struct CCOMTypeCast {
		union {
			int lLetters;
			int bLetters;
		};
		int lTypeLen;
		CCOMType eCComType;
		int eLen;
		int eCComFormat;
	};

	struct CComRead {
		const CSharedCom* m_pCom;
		const char* m_pName;

		operator bool() const;
		operator char() const;
		operator int() const;
		operator float() const;
		operator char* () const;
		operator int* () const;
		operator float* () const;
		operator unsigned int() const;
	};

	struct ComValueInfo {
		int lNameLen;
		CCOMTypeCast* lType;
		int lDataLen;
	};

	struct CGlobalComMemInfo {
		int size;
		bool used;
		bool pad[3];
	};


	// Size: 0x4 (4 bytes)
	struct CSharedCom {
		virtual ~CSharedCom();
		virtual void Clear();                            // vtable [1]
		virtual void RemoveVal(const char* key, int id); // vtable [2]
		virtual bool Exists(const char* key, int id) const; // vtable [3]

		virtual void SetVal(const char* key, bool val, CCOMType type);
		virtual void SetVal(const char* key, char val, CCOMType type);
		virtual void SetVal(const char* key, unsigned int val, CCOMType type);
		virtual void SetVal(const char* key, int val, CCOMType type);
		virtual void SetVal(const char* key, float val, CCOMType type);
		virtual void SetVal(const char* key, bool val);
		virtual void SetVal(const char* key, char val);
		virtual void SetVal(const char* key, int val);
		virtual void SetVal(const char* key, float val);
		virtual void SetVal(const char* key, REFTAB* val, CCOMType type);
		virtual void SetVal(const char* key, const char* val, CCOMType type);
		virtual void SetVal(const char* key, const int* val, CCOMType type);
		virtual void SetVal(const char* key, const char* val, int size, CCOMType type);
		virtual void SetVal(const char* key, const int* val, int size, CCOMType type);
		virtual void SetVal(const char* key, const float* val, int size, CCOMType type);
		virtual void SetVal(const char* key, const char* val, int size);
		virtual void SetVal(const char* key, const int* val, int size);
		virtual void SetVal(const char* key, const float* val, int size);
		virtual void SetVal(const char* key, const char* val);
		virtual void SetVal(const char* key, const int* val);
		virtual void SetVal(const char* key, const float* val);

		virtual int GetVal(const char* key, bool* outVal) const;
		virtual int GetVal(const char* key, char* outVal) const;
		virtual int GetVal(const char* key, int* outVal) const;
		virtual int GetVal(const char* key, float* outVal) const;
		virtual void* Get(const char* key) const;
		virtual void* operator[](const char* key) const;
		virtual int GetVal(const char* key, bool** outPtr) const;
		virtual int GetVal(const char* key, char** outPtr) const;
		virtual int GetVal(const char* key, int** outPtr) const;
		virtual int GetVal(const char* key, float** outPtr) const;
		virtual char* GetVal(const char* key) const;
		virtual char* GetValPtr(const char* key, CCOMFormat format) const;

		virtual int GetDataLen(const char* key) const;
		virtual void SetVal(const char* key, int val, const char* subKey, int unk);
		virtual int GetVal(char* outBuffer, const char* key, int size) const;

		virtual void AddValOfType(const char* key, CCOMFormat format, CCOMType type, const char* val, int size); // vtable [41]
		virtual ComValueInfo* GetpVal(const char* key, int id) const; // vtable [42]
	};

	struct CGlobalCom : public CSharedCom {
		int maxSize;
		void* memory;
		int memoryWritePtr;
	};

	struct CCom : public CSharedCom 
	{
		char m_Memory[16384];
		void* m_pMemory;
		REFTAB32 m_ComDat;
		int m_OffsetAllocLinks[16];
		ZOffsetAlloc m_OffsetAlloc;
	};
}