#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/ZUniAssert.h>
#include <memory>

namespace Glacier
{
    CGlobalCom::CGlobalCom(void* pBuffer, int lBufferSize)
    {
        maxSize = lBufferSize;
        memory = pBuffer;
        memoryWritePtr = 0;
    }

    CGlobalCom::CGlobalCom()
    {
        maxSize = 0;
        memory = nullptr;
        memoryWritePtr = 0;
    }

    CGlobalCom::~CGlobalCom() = default;

    void CGlobalCom::Clear()
    {
        memoryWritePtr = 0;
    }

    bool CGlobalCom::RemoveVal(const char* pValName, int lValNameLen)
    {
        auto* pFound = Find(pValName, lValNameLen);
        if (pFound)
        {
            pFound->used = false;
            return true;
        }

        return false;
    }

    bool CGlobalCom::Exists(const char* pValName, int lValNameLen)
    {
        return Find(pValName, lValNameLen) != nullptr;
    }

    void CGlobalCom::AddValOfType(const char* pValName, int lValNameLen, CCOMTypeCast* CCTCType, int lDataLen, const char* pData)
    {
        const size_t lTotalSize = lValNameLen + sizeof(ComValueInfo) + lDataLen;
        uint8_t* pBuffer = static_cast<uint8_t*>(alloca(lTotalSize));

        char* pNameBuffer = reinterpret_cast<char*>(pBuffer + sizeof(ComValueInfo));
        ComValueInfo* pHeader = reinterpret_cast<ComValueInfo*>(pBuffer);
        void* pDataBuffer = reinterpret_cast<void*>(pBuffer + sizeof(ComValueInfo) + lValNameLen);

        if (pValName && lValNameLen > 0)
        {
            memcpy(pNameBuffer, pValName, lValNameLen);
        }

        if (pData && lDataLen > 0)
        {
            memcpy(pDataBuffer, pData, lDataLen);
        }

        pHeader->lNameLen = lValNameLen;
        pHeader->lType = CCTCType;
        pHeader->lDataLen = lDataLen;

        Append(pHeader);
    }
	
    ComValueInfo* CGlobalCom::GetpVal(const char* pValName, int lValNameLen)
    {
        auto* pFound = Find(pValName, lValNameLen);
        if (!pFound)
            return nullptr;
        
        return reinterpret_cast<ComValueInfo*>(reinterpret_cast<uint8_t*>(pFound) + sizeof(CGlobalComMemInfo));
    }

    bool CGlobalCom::EnumKeys(char* output, int maxLen, unsigned int index)
    {
        if (!memoryWritePtr)
            return false;

        uint32_t currentOffset = 0;

        while (currentOffset < static_cast<uint32_t>(memoryWritePtr))
        {
            auto* pMemInfo = reinterpret_cast<CGlobalComMemInfo*>(static_cast<uint8_t*>(memory) + currentOffset);
            uint32_t entrySize = pMemInfo->size;
            
            currentOffset += entrySize;

            if (pMemInfo->used)
            {
                if (index > 0)
                {
                    --index;
                    continue;
                }

                ComValueInfo* pValueInfo = reinterpret_cast<ComValueInfo*>(pMemInfo + 1);
                int nameLen = pValueInfo->lNameLen;
                if (nameLen + 1 > maxLen)
                {
                    ZASSERT(false);
                    return false;
                }

                const char* pValName = reinterpret_cast<const char*>(pValueInfo + 1);

                memcpy(output, pValName, nameLen);
                output[nameLen] = '\0';

                return true;
            }
        }
        
        return false;
    }

    void CGlobalCom::Append(ComValueInfo* pVal)
    {
        uint32_t lEntrySize = pVal->lDataLen + pVal->lNameLen + sizeof(CGlobalComMemInfo) + sizeof(ComValueInfo);
        const char* pValName = reinterpret_cast<const char*>(pVal + 1);
        CGlobalComMemInfo* pExistingHeader = Find(pValName, pVal->lNameLen);

        if (pExistingHeader)
        {
            if (pExistingHeader->size == lEntrySize)
            {
                ComValueInfo* pDestination = reinterpret_cast<ComValueInfo*>(pExistingHeader + 1);
                memcpy(pDestination, pVal, lEntrySize - sizeof(CGlobalComMemInfo));
                return;
            }

            pExistingHeader->used = false;
        }

        if (memoryWritePtr + lEntrySize > maxSize)
        {
            // Try to free some mem
            Defrag();

            if (memoryWritePtr + lEntrySize > maxSize)
            {
                ZASSERT(false);
                return;
            }
        }

        auto* pNewHeader = reinterpret_cast<CGlobalComMemInfo*>(static_cast<uint8_t*>(memory) + memoryWritePtr);
        memoryWritePtr += lEntrySize;

        pNewHeader->size = lEntrySize;
        pNewHeader->used = true;

        ComValueInfo* pDestination = reinterpret_cast<ComValueInfo*>(pNewHeader + 1);
        uint32_t bytesToCopy = lEntrySize - sizeof(CGlobalComMemInfo);
        
        memcpy(pDestination, pVal, bytesToCopy);
    }

    void CGlobalCom::Defrag()
    {
        uint32_t writeOffset = 0;
        uint32_t readOffset = 0;

        while (readOffset < static_cast<uint32_t>(memoryWritePtr))
        {
            auto* pCurrentHeader = reinterpret_cast<CGlobalComMemInfo*>(static_cast<uint8_t*>(memory) + readOffset);

            uint32_t entrySize = pCurrentHeader->size;
            readOffset += entrySize;

            if (pCurrentHeader->used)
            {
                uint8_t* pDestination = static_cast<uint8_t*>(memory) + writeOffset;                
                writeOffset += entrySize;

                if (readOffset != writeOffset)
                {
                    memmove(pDestination, pCurrentHeader, entrySize);
                }
            }
        }

        memoryWritePtr = writeOffset;
    }

    CGlobalCom::CGlobalComMemInfo* CGlobalCom::Find(const char* pValName, int lValNameLen)
    {
        size_t lNameLen = lValNameLen;
        if (lNameLen == 0)
        {
            if (!pValName)
                return nullptr;
            
            lNameLen = strlen(pValName) + 1;
        }

        if (memoryWritePtr == 0)
            return nullptr;

        uint32_t currentOffset = 0;
        while (true)
        {
            auto* pMemInfo = reinterpret_cast<CGlobalComMemInfo*>(static_cast<uint8_t*>(memory) + currentOffset);

            uint32_t entrySize = pMemInfo->size;
            bool isUsed = pMemInfo->used;
            currentOffset += entrySize;

            if (isUsed)
            {
                ComValueInfo* pValueInfo = reinterpret_cast<ComValueInfo*>(pMemInfo + 1);
                const char* pCurrentName = reinterpret_cast<const char*>(pValueInfo + 1);

                if (pValueInfo->lNameLen == static_cast<int>(lNameLen) && memicmp(pValName, pCurrentName, lNameLen) == 0)
                {
                    // Found!
                    return pMemInfo;
                }
            }

            if (currentOffset >= static_cast<uint32_t>(memoryWritePtr))
                return nullptr;
        }
    }

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(CGlobalCom*, g_pGlobalCOM, 0x00820830, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(CGlobalCom, g_GlobalCom, 0x008B4B90, {});
}