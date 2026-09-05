#include <Glacier/Com/CCom.h>
#include <Glacier/Com/CComRead.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    CCom::CCom()
        : m_ComDat()
        , m_OffsetAlloc(0, 0x4000, 0x10, &m_OffsetAllocLinks[0], EAllocType::DEFAULT_MEM)
    {
        m_pMemory = m_Memory;
    }

    CCom::~CCom()
    {
        m_OffsetAlloc.Reset(0, 0x4000);
        Clear();
    }

    void CCom::Clear()
    {
        m_OffsetAlloc.Reset(0, 0x4000);
        m_ComDat.Clear();
        m_pMemory = m_Memory;
    }

    bool CCom::RemoveVal(const char* pValName, int lValNameLen)
    {
        if (!pValName)
        {
            return false;
        }

        if (lValNameLen == 0)
        {
            lValNameLen = static_cast<int>(strlen(pValName)) + 1;
        }

        for (ComValueInfo* pInfo : m_ComDat.As<ComValueInfo*>())
        {
            if (!pInfo)
            {
                continue;
            }

            if (CSharedCom::SameName(pValName, lValNameLen, pInfo))
            {
                Free(pInfo);

                m_ComDat.Remove(reinterpret_cast<uint32_t>(pInfo));
                return true;
            }
        }

        return false;
    }

    bool CCom::Exists(const char* pValName, int lValNameLen)
    {
        if (!pValName)
        {
            return false;
        }

        if (lValNameLen == 0)
        {
            lValNameLen = static_cast<int>(strlen(pValName)) + 1;
        }

        for (ComValueInfo* pInfo : m_ComDat.As<ComValueInfo*>())
        {
            if (pInfo && CSharedCom::SameName(pValName, lValNameLen, pInfo))
            {
                return true;
            }
        }

        return false;
    }

    void CCom::AddValOfType(const char* pValName, int lValNameLen, CCOMTypeCast* CCTCType, int lDataLen, const char* pData)
    {
        RemoveVal(pValName, 0);

        uint32_t totalSize = lValNameLen + lDataLen + sizeof(ComValueInfo);

        ComValueInfo* pInfo = reinterpret_cast<ComValueInfo*>(Alloc(totalSize));
        if (!pInfo)
            return;

        pInfo->lNameLen  = lValNameLen;
        pInfo->lType = CCTCType;
        pInfo->lDataLen  = lDataLen;

        char* pDstName = reinterpret_cast<char*>(pInfo + 1);
        memcpy(pDstName, pValName, lValNameLen);

        void* pDstData = pDstName + lValNameLen;
        if (pData && lDataLen > 0)
        {
            memcpy(pDstData, pData, lDataLen);
        }

        m_ComDat.Add(reinterpret_cast<uint32_t>(pInfo));
    }
    
    void CCom::CopyAll(CCom* pSource)
    {
        if (!pSource)
            return;

        for (ComValueInfo* pSrcInfo : pSource->m_ComDat.As<ComValueInfo*>())
        {
            if (!pSrcInfo)
                continue;

            uint32_t recordSize = sizeof(ComValueInfo) + pSrcInfo->lNameLen + pSrcInfo->lDataLen;
            uint32_t alignedBlockSize = (recordSize + 7) & ~3U;
            uint32_t offset = reinterpret_cast<uint32_t>(m_OffsetAlloc.Alloc(alignedBlockSize, 1));
            uint32_t* pBlockHeader = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(m_pMemory) + offset);
            *pBlockHeader = alignedBlockSize;

            ComValueInfo* pDstInfo = reinterpret_cast<ComValueInfo*>(pBlockHeader + 1);
            memcpy(pDstInfo, pSrcInfo, recordSize);

            m_ComDat.Add(reinterpret_cast<uint32_t>(pDstInfo));
        }
    }

    void CCom::Free(void* pData)
    {
        if (!pData)
            return;

        uint32_t* pUserPtr = static_cast<uint32_t*>(pData);

        uint32_t totalAlignedSize = *(pUserPtr - 1);

        uintptr_t headerAddress = reinterpret_cast<uintptr_t>(pUserPtr - 1);
        uintptr_t baseAddress   = reinterpret_cast<uintptr_t>(m_pMemory);
        
        uint32_t offset = static_cast<uint32_t>(headerAddress - baseAddress);

        m_OffsetAlloc.Free(offset, totalAlignedSize);
    }

    void CCom::PrintStatus()
    {
        printf("CCom::PrintStatus\n");
        if (m_ComDat.Count() == 0)
        {
            printf("(Empty)\n");
            return;
        }

        for (ComValueInfo* pInfo : m_ComDat.As<ComValueInfo*>())
        {
            if (!pInfo)
                continue;

            const char* pValName = reinterpret_cast<const char*>(pInfo + 1);
            CComRead reader(this, pValName);

            switch (pValName[0])
            {
                case 'b':
                    printf("  [BOOL]  %-24s = %s\n", pValName, (bool)reader ? "true" : "false");
                    break;
                case 'f':
                    printf("  [FLOAT] %-24s = %.4f\n", pValName, (float)reader);
                    break;
                case 'i':
                    printf("  [INT]   %-24s = %d\n", pValName, (int32_t)reader);
                    break;
                case 's':
                    printf("  [STR]   %-24s = \"%s\"\n", pValName, (char*)reader ? (char*)reader : "null");
                    break;
                default:
                    printf("  [RAW]   %-24s (Size: %d B)\n", pValName, pInfo->lDataLen);
                    break;
            }
        }
    }

    void* CCom::Alloc(uint32_t lSize)
    {
        uint32_t totalAlignedSize = (lSize + 7) & ~3U;
        uint32_t offset = reinterpret_cast<uint32_t>(m_OffsetAlloc.Alloc(totalAlignedSize, 1));

        uint32_t* pBlockHeader = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(m_pMemory) + offset);
        *pBlockHeader = totalAlignedSize;

        return pBlockHeader + 1;
    }

    ComValueInfo* CCom::GetpVal(const char* pValName, int lValNameLen)
    {
        if (!pValName)
        return nullptr;

        if (lValNameLen == 0)
            lValNameLen = static_cast<int>(strlen(pValName)) + 1;

        for (ComValueInfo* pInfo : m_ComDat.As<ComValueInfo*>())
        {
            if (pInfo && CSharedCom::SameName(pValName, lValNameLen, pInfo))
            {
                return pInfo;
            }
        }

        return nullptr;
    }
}