#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/CHUNKFILE.h>
#include <cstdint>
#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr int kAnimTemplatesNamesChunk = 7;

        RE_PACKED_STRUCT(1)
        struct ZAnimTemplatesNamesEntry
        {
            int32_t nameOffset;
            int16_t animId;
            int16_t padding;
        } RE_PACKED_STRUCT_END;
        RE_VERIFY_SIZE(ZAnimTemplatesNamesEntry, 0x8);

        const char* GetStringBase(const char* pBuffer)
        {
            const int32_t count = *reinterpret_cast<const int32_t*>(pBuffer);
            return pBuffer + sizeof(int32_t) + count * sizeof(ZAnimTemplatesNamesEntry);
        }

        const ZAnimTemplatesNamesEntry* GetEntries(const char* pBuffer)
        {
            return reinterpret_cast<const ZAnimTemplatesNamesEntry*>(pBuffer + sizeof(int32_t));
        }
    }

    ZAnimTemplatesNames::ZAnimTemplatesNames() = default;

    bool ZAnimTemplatesNames::FindAnimVariationHandle(ZAnimVariationHandle& handle, const char* pszName)
    {
        const char* pBuffer = m_pBuffer;
        if (!pBuffer)
        {
            return false;
        }

        const int32_t count = *reinterpret_cast<const int32_t*>(pBuffer);
        const ZAnimTemplatesNamesEntry* entries = GetEntries(pBuffer);
        const char* strings = GetStringBase(pBuffer);

        int32_t lo = 0;
        int32_t hi = count;

        while (lo < hi)
        {
            const int32_t mid = (hi + lo) / 2;
            const int cmp = std::strcmp(pszName, strings + entries[mid].nameOffset);

            if (cmp == 0)
            {
                handle.iIndex = entries[mid].animId;
                return true;
            }

            if (cmp < 0)
            {
                hi = mid;
            }
            else
            {
                lo = mid + 1;
            }
        }

        return false;
    }

    void ZAnimTemplatesNames::SetBuffer(const char* pBuffer)
    {
        m_pBuffer = pBuffer;
    }

    bool ZAnimTemplatesNames::Init()
    {
        CHUNKFILE* packedAnims = g_pSysInterface->m_pEngineData->m_pPackedAnims;
        if (!packedAnims)
        {
            return false;
        }

        CHUNKFILE* child = packedAnims->FindChild(kAnimTemplatesNamesChunk);
        if (!child)
        {
            return false;
        }

        SetBuffer(static_cast<const char*>(child->Data()));
        return true;
    }

    int32_t ZAnimTemplatesNames::GetAnimCount() const
    {
        if (!m_pBuffer)
        {
            return 0;
        }

        return *reinterpret_cast<const int32_t*>(m_pBuffer);
    }

    bool ZAnimTemplatesNames::GetAnim(int index, ZAnimVariationHandle& handle)
    {
        if (index < 0 || index >= GetAnimCount())
        {
            return false;
        }

        handle.iIndex = GetEntries(m_pBuffer)[index].animId;
        return true;
    }

    const char* ZAnimTemplatesNames::GetNameFromAnimVariationHandle(const ZAnimVariationHandle& handle)
    {
        const char* pBuffer = m_pBuffer;
        if (!pBuffer)
        {
            return nullptr;
        }

        const int32_t count = *reinterpret_cast<const int32_t*>(pBuffer);
        const ZAnimTemplatesNamesEntry* entries = GetEntries(pBuffer);
        const char* strings = GetStringBase(pBuffer);

        for (int32_t i = 0; i < count; ++i)
        {
            if (handle.iIndex == entries[i].animId)
            {
                return strings + entries[i].nameOffset;
            }
        }

        return nullptr;
    }
}
