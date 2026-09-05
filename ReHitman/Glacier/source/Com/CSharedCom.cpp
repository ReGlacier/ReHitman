#include <Glacier/Com/CCOMTypeCast.h>
#include <Glacier/Com/CSharedCom.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <memory>
#include <cstdio>


namespace Glacier
{
    /*
    Common memory layout:

    +-------------------+--------------------+-----------------------+
    |  ComValueInfo     |  Name (pValName)   |  Data (outBuffer)     |
    |  (sizeof = 16 B)  |  (lNameLen B)      |  (lDataLen B)         |
    +-------------------+--------------------+-----------------------+
    ^                   ^                    ^
    |                   |                    |
    pInfo             pInfo + 1          pDataSrc = (pInfo + 1) + lNameLen
    */

    // Globals
    using CCOMTypeTable_t = CCOMTypeCast[18];
    STATIC_GLOBAL_CLASS_INSTANCE(CCOMTypeTable_t, CCOMTypeTable);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(CCOMTypeTable_t, CCOMTypeTable, 0x007F2770, {});

    CSharedCom::~CSharedCom() = default;

    void CSharedCom::SetVal(const char* pValName, bool Val, CCOMType CCT)
    {
        auto copy = Val;
        AddValOfType(pValName, CCOMFormat::CCOM_FORMAT_BOOL, CCT, &copy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, char Val, CCOMType CCT)
    {
        auto copy = Val;
        AddValOfType(pValName, CCOMFormat::CCOM_FORMAT_CHAR, CCT, &copy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, uint32_t Val, CCOMType CCT)
    {
        uint32_t valCopy = Val;
        AddValOfType(pValName, CCOMFormat::CCOM_FORMAT_int32, CCT, &valCopy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, int Val, CCOMType CCT)
    {
        int valCopy = Val;
        AddValOfType(pValName, CCOM_FORMAT_int32, CCT, &valCopy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, float Val, CCOMType CCT)
    {
        float valCopy = Val;
        AddValOfType(pValName, CCOM_FORMAT_FLOAT, CCT, &valCopy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, bool Val)
    {
        bool valCopy = Val;
        AddVal(pValName, CCOM_FORMAT_BOOL, &valCopy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, char Val)
    {
        char valCopy = Val;
        AddVal(pValName, CCOM_FORMAT_CHAR, &valCopy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, int Val)
    {
        int valCopy = Val;
        AddVal(pValName, CCOM_FORMAT_int32, &valCopy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, float Val)
    {
        float valCopy = Val;
        AddVal(pValName, CCOM_FORMAT_FLOAT, &valCopy, 0);
    }

    void CSharedCom::SetVal(const char* pValName, REFTAB* pRefTab, CCOMType CCT)
    {
        if (!pRefTab)
        {
            AddValOfType(pValName, CCOM_FORMAT_int32, CCT, nullptr, 0);
            return;
        }
        
        const int count = pRefTab->Count();
        uint32_t* buffer = static_cast<uint32_t*>(alloca((count + 1) * sizeof(uint32_t)));
        buffer[0] = static_cast<uint32_t>(count);

        int index = 1;
        for (uint32_t ref : *pRefTab)
        {
            buffer[index++] = ref;
        }

        AddValOfType(pValName, CCOM_FORMAT_int32, CCT, buffer, 0);
    }

    void CSharedCom::SetVal(const char* pValName, const char* Val, CCOMType CCT)
    {
        AddValOfType(pValName, CCOM_FORMAT_CHAR, CCT, Val, 0);
    }

    void CSharedCom::SetVal(const char* pValName, const int* Val, CCOMType CCT)
    {
        AddValOfType(pValName, CCOM_FORMAT_int32, CCT, Val, 0);
    }

    void CSharedCom::SetVal(const char* pValName, const char* Val, int lNrValues, CCOMType CCT)
    {
        AddValOfType(pValName, CCOM_FORMAT_CHAR, CCT, Val, sizeof(char) * lNrValues);
    }

    void CSharedCom::SetVal(const char* pValName, const int* Val, int lNrValues, CCOMType CCT)
    {
        AddValOfType(pValName, CCOM_FORMAT_int32, CCT, Val, sizeof(int) * lNrValues);
    }

    void CSharedCom::SetVal(const char* pValName, const float* Val, int lNrValues, CCOMType CCT)
    {
        AddValOfType(pValName, CCOM_FORMAT_FLOAT, CCT, Val, sizeof(float) * lNrValues);
    }

    void CSharedCom::SetVal(const char* pValName, const char* Val, int lNrVals)
    {
        AddVal(pValName, CCOM_FORMAT_CHAR, (const void*)Val, sizeof(char) * lNrVals);
    }

    void CSharedCom::SetVal(const char* pValName, const int* Val, int lNrVals)
    {
        AddVal(pValName, CCOM_FORMAT_int32, (const void*)Val, sizeof(int) * lNrVals);
    }

    void CSharedCom::SetVal(const char* pValName, const float* Val, int lNrVals)
    {
        AddVal(pValName, CCOM_FORMAT_FLOAT, (const void*)Val, sizeof(float) * lNrVals);
    }

    void CSharedCom::SetVal(const char* pValName, const char* Val)
    {
        AddVal(pValName, CCOM_FORMAT_CHAR, (const void*)Val, 0);
    }

    void CSharedCom::SetVal(const char* pValName, const int* Val)
    {
        AddVal(pValName, CCOM_FORMAT_int32, (const void*)Val, 0);
    }

    void CSharedCom::SetVal(const char* pValName, const float* Val)
    {
        AddVal(pValName, CCOM_FORMAT_FLOAT, (const void*)Val, 0);
    }

    int32_t CSharedCom::GetVal(const char* pValName, bool* pVal)
    {
        if (!pVal)
            return 0;

        int32_t tempVal = *pVal;
        int32_t bytesRead = GetVal(reinterpret_cast<char*>(&tempVal), pValName, 0);
        
        *pVal = (tempVal != 0);
        return bytesRead;
    }

    int32_t CSharedCom::GetVal(const char* pValName, char* pVal)
    {
        return GetVal(pVal, pValName, 0);
    }

    int32_t CSharedCom::GetVal(const char* pValName, int* pVal)
    {
        return GetVal(reinterpret_cast<char*>(pVal), pValName, 0);
    }

    int32_t CSharedCom::GetVal(const char* pValName, float* pVal)
    {
        return GetVal(reinterpret_cast<char*>(pVal), pValName, 0);
    }

    CComRead CSharedCom::Get(char const* pValName)
    {
        return CComRead(this, pValName);
    }

    CComRead CSharedCom::operator[](char const* pValName)
    {
        return CComRead(this, pValName); 
    }
    
    int32_t CSharedCom::GetVal(const char* pValName, bool** ppOutVal)
    {
        return GetVal(reinterpret_cast<char*>(ppOutVal), pValName, CCOM_FORMAT_BOOL);
    }

    int32_t CSharedCom::GetVal(const char* pValName, char** ppOutVal)
    {
        return GetVal(reinterpret_cast<char*>(ppOutVal), pValName, CCOM_FORMAT_CHAR);
    }

    int32_t CSharedCom::GetVal(const char* pValName, int** ppOutVal)
    {
        return GetVal(reinterpret_cast<char*>(ppOutVal), pValName, CCOM_FORMAT_int32);
    }

    int32_t CSharedCom::GetVal(const char* pValName, float** ppOutVal)
    {
        return GetVal(reinterpret_cast<char*>(ppOutVal), pValName, CCOM_FORMAT_FLOAT);
    }
    
    const char* CSharedCom::GetVal(const char* pValName)
    {
        return GetValPtr(pValName, CCOM_FORMAT_CHAR);
    }

    const char* CSharedCom::GetValPtr(const char* pValName, CCOMFormat eFormat)
    {
        ComValueInfo* pInfo = GetpVal(pValName, 0);

        if (pInfo)
        {
            const char* pDataPtr = reinterpret_cast<const char*>(pInfo + 1) + pInfo->lNameLen;
            return const_cast<char*>(pDataPtr);
        }

        return nullptr;
    }

    int32_t CSharedCom::GetDataLen(const char* pValName)
    {
        auto* pVal = GetpVal(pValName, 0);
        if (pVal)
        {
            return pVal->lDataLen;
        }

        return 0;
    }
    
    void CSharedCom::SetVal(const char *pValName, int lValNameLen, const char *Val, const int lLen)
    {
        AddValOfType(pValName, lValNameLen, CCOM_TYPE_BLOCK, lLen, Val);
    }

    int32_t CSharedCom::GetVal(char* outBuffer, const char* pValName, int lValNameLen)
    {
        auto* pVal = GetpVal(pValName, lValNameLen);

        if (pVal)
        {
            const char* pDataSrc = reinterpret_cast<const char*>(pVal + 1) + pVal->lNameLen;
            memcpy(outBuffer, pDataSrc, pVal->lDataLen);
            return pVal->lDataLen;
        }

        return 0;
    }

    bool CSharedCom::SameName(const char* pValName, int lValNameLen, ComValueInfo* pInfo) const
    {
        if (!pInfo || !pValName)
            return false;

        if (pInfo->lNameLen != lValNameLen)
            return false;

        const char* pValueName = reinterpret_cast<const char*>(pInfo + 1);
        return _memicmp(pValName, pValueName, lValNameLen) == 0;
    }

    void CSharedCom::AddVal(const char* pValName, CCOMFormat eFormat, const void* pData, int lDataLen)
    {
        CCOMTypeCast* pTypeCast = this->GetTypeCast(pValName);

        if ((pTypeCast->eCComFormat & eFormat) == 0)
        {
            printf("CSharedCom SetVal error: %s doesn't match type given", pValName);
            ZASSERT(false);
            return;
        }

        const void* pActualData = pData;

        if (pTypeCast->eComType == CCOM_TYPE_PTR)
        {
            pActualData = &pData;
        }

        int lNameLen = static_cast<int>(strlen(pValName) + 1);

        if (lDataLen == 0)
        {
            lDataLen = pTypeCast->eLen;

            if (lDataLen == 0)
            {
                switch (pTypeCast->eComType)
                {
                    case CCOM_TYPE_STRING:
                    case CCOM_TYPE_FILE:
                    {
                        lDataLen = static_cast<int>(strlen(static_cast<const char*>(pActualData)) + 1);
                        break;
                    }

                    case CCOM_TYPE_DATA:
                    case CCOM_TYPE_GEOMREFTAB:
                    {
                        lDataLen = *static_cast<const int*>(pActualData);
                        break;
                    }

                    default:
                    {
                        printf("CSharedCom error!!\n");
                        ZASSERT(false);
                        break;
                    }
                }
            }
        }

        AddValOfType(
            pValName, 
            lNameLen, 
            pTypeCast->eComType,
            lDataLen, 
            static_cast<const char*>(pActualData)
        );
    }

    CCOMTypeCast* CSharedCom::GetTypeCast(const char* pTypeName) const
    {
        if (!pTypeName)
            return nullptr;

        int maxMatchLength = 0;
        int bestMatchedIndex = -1;

        for (int i = 0; i < 18; ++i)
        {
            const CCOMTypeCast* pType = &CCOMTypeTable[i];
            int charIdx = 0;

            while (charIdx < pType->lTypeLen)
            {
                if (pTypeName[charIdx] != static_cast<char>(pType->bLetters[3 - static_cast<int>(charIdx)]))
                {
                    break;
                }
                ++charIdx;
            }

            if (charIdx == pType->lTypeLen && charIdx > maxMatchLength)
            {
                maxMatchLength = charIdx;
                bestMatchedIndex = i;
            }
        }

        if (bestMatchedIndex == -1)
            return nullptr;

        return &CCOMTypeTable[bestMatchedIndex];
    }

    CCOMTypeCast* CSharedCom::GetTypeCast(CCOMType eType) const
    {
        for (int i = 0; i < 18; ++i)
        {
            if (CCOMTypeTable[i].eComType == eType)
            {
                return &CCOMTypeTable[i];
            }
        }

        return nullptr;
    }

    void CSharedCom::AddValOfType(const char* pValName, CCOMFormat eFormat, CCOMType eComType, const void* pData, int lDataLen)
    {
        CCOMTypeCast* pTypeCast = GetTypeCast(eComType);

        if ((pTypeCast->eCComFormat & eFormat) == 0)
        {
            printf("CSharedCom SetVal error: %s doesn't match type given", pValName);
            ZASSERT(false);
            return;
        }

        const void* pActualData = pData;

        if (pTypeCast->eComType == CCOM_TYPE_PTR)
        {
            pActualData = &pData;
        }

        int lNameLen = static_cast<int>(strlen(pValName) + 1);
        if (lDataLen == 0)
        {
            lDataLen = pTypeCast->eLen;

            if (lDataLen == 0)
            {
                switch (pTypeCast->eComType)
                {
                    case CCOM_TYPE_STRING: // 12
                    case CCOM_TYPE_FILE:   // 14
                    {
                        lDataLen = static_cast<int>(strlen(static_cast<const char*>(pActualData)) + 1);
                        break;
                    }

                    case CCOM_TYPE_DATA:       // 15
                    case CCOM_TYPE_GEOMREFTAB: // 17
                    {
                        lDataLen = *static_cast<const int*>(pActualData);
                        break;
                    }

                    default:
                    {
                        printf("CSharedCom error!!\n");
                        ZASSERT(false);
                        break;
                    }
                }
            }
        }

        AddValOfType(
            pValName, 
            lNameLen, 
            pTypeCast->eComType,
            lDataLen, 
            static_cast<const char*>(pActualData)
        );
    }

    void CSharedCom::AddValOfType(const char* pValName, int lValNameLen, CCOMType eComType, int lDataLen, const char* pData)
    {
        CCOMTypeCast* pTypeCast = this->GetTypeCast(eComType);
        AddValOfType(pValName, lValNameLen, pTypeCast, lDataLen, pData);
    }
}