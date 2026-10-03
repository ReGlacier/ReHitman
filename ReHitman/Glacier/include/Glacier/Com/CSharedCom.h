#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Com/CCOMType.h>
#include <Glacier/Com/CCOMFormat.h>
#include <Glacier/Com/CCOMTypeCast.h>
#include <Glacier/Com/CComRead.h>
#include <Glacier/Com/ComValueInfo.h>

namespace Glacier
{
    // fwds
    class REFTAB;

    struct CSharedCom
	{
		// vtbl
		virtual ~CSharedCom();
		virtual void Clear(void) = 0;
		virtual void SetVal(const char* pValName, bool Val, CCOMType CCT);
		virtual void SetVal(const char* pValName, char Val, CCOMType CCT);
		virtual void SetVal(const char* pValName, uint32_t Val, CCOMType CCT);
		virtual void SetVal(const char* pValName, int Val, CCOMType CCT);
		virtual void SetVal(const char* pValName, float Val, CCOMType CCT);
		virtual void SetVal(const char* pValName, bool Val);
		virtual void SetVal(const char* pValName, char Val);
		virtual void SetVal(const char* pValName, int Val);
		virtual void SetVal(const char* pValName, float Val);
		virtual void SetVal(const char* pValName, REFTAB* pRefTab, CCOMType CCT);
		virtual void SetVal(const char* pValName, const char* Val, CCOMType CCT);
		virtual void SetVal(const char* pValName, const int* Val, CCOMType CCT);
		virtual void SetVal(const char* pValName, const char* Val, int lNrValues, CCOMType CCT);
		virtual void SetVal(const char* pValName, const int* Val, int lNrValues, CCOMType CCT);
		virtual void SetVal(const char* pValName, const float* Val, int lNrValues, CCOMType CCT);
		virtual void SetVal(const char* pValName, const char* Val, int lNrVals);
		virtual void SetVal(const char* pValName, const int* Val, int lNrVals);
		virtual void SetVal(const char* pValName, const float* Val, int lNrVals);
		virtual void SetVal(const char* pValName, const char* Val);
		virtual void SetVal(const char* pValName, const int* Val);
		virtual void SetVal(const char* pValName, const float* Val);
		virtual int32_t GetVal(const char* pValName, bool* pVal);
		virtual int32_t GetVal(const char* pValName, char* pVal);
		virtual int32_t GetVal(const char* pValName, int* pVal);
		virtual int32_t GetVal(const char* pValName, float* pVal);
		virtual CComRead Get(char const* pValName);
		virtual CComRead operator[](char const* pValName);
		virtual int32_t GetVal(const char* pValName, bool ** ppOutVal);
		virtual int32_t GetVal(const char* pValName, char ** ppOutVal);
		virtual int32_t GetVal(const char* pValName, int ** ppOutVal);
		virtual int32_t GetVal(const char* pValName, float ** ppOutVal);
		virtual const char* GetVal(const char* pValName);
		virtual const char* GetValPtr(const char* pValName, CCOMFormat eFormat);
		virtual bool RemoveVal(const char* pValName, int lValNameLen) = 0;
		virtual bool Exists(const char* pValName, int lValNameLen) = 0;
		virtual int32_t GetDataLen(const char* pValName);
		virtual void SetVal(const char *pValName, int lValNameLen, const char *Val, const int lLen);
		virtual int32_t GetVal(char* outBuffer, const char* pValName, int lValNameLen);
		virtual void AddValOfType(const char* pValName, int lValNameLen, CCOMTypeCast* CCTCType, int lDataLen, const char* pData) = 0;
		virtual ComValueInfo* GetpVal(const char* pValName, int lValNameLen) = 0;

		// members
		bool SameName(const char* pValName, int lValNameLen, ComValueInfo* pInfo) const;
		void AddVal(const char* pValName, CCOMFormat eFormat, const void* pData, int lDataLen);

		CCOMTypeCast* GetTypeCast(const char*) const;
		CCOMTypeCast* GetTypeCast(CCOMType eType) const;
		void AddValOfType(const char* pValName, CCOMFormat eFormat, CCOMType eComType, const void* pData, int lDataLen);
		void AddValOfType(const char* pValName, int lValNameLen, CCOMType eComType, int lDataLen,  const char* pData);
	};
	RE_VERIFY_SIZE(CSharedCom, 0x4);
}