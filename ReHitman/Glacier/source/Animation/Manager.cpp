#include <Glacier/Animation/Manager.h>
#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/ZHumanState.h>
#include <Glacier/CHUNKFILE.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <cstddef>
#include <cstdint>
#include <cstring>


namespace Glacier::Animation
{
    namespace
    {
        constexpr size_t kAnimCountOffset = 0x00;
        constexpr size_t kDataSizeOffset = 0x04;
        constexpr size_t kBoneNamesSizeOffset = 0x08;
        constexpr size_t kAnimNamesSizeOffset = 0x0C;
        constexpr size_t kPoseNamesSizeOffset = 0x10;
        constexpr size_t kBoneNamesCountOffset = 0x14;
        constexpr size_t kAnimNamesCountOffset = 0x18;
        constexpr size_t kPoseNamesCountOffset = 0x1C;
        constexpr size_t kUncompressedDataSizeOffset = 0x20;
        constexpr size_t kStateSizeOffset = 0x24;
        constexpr size_t kHumanQuatSizeOffset = 0x28;
        constexpr size_t kQuatSizeOffset = 0x2C;
        constexpr size_t kPoseSizeOffset = 0x30;
        constexpr size_t kHeaderArrayOffset = 0x34;

        constexpr std::uintptr_t kPointerAlignmentMask = 1;
        constexpr int kMetaKeyDataChunk = 8;
        constexpr int kMetaKeyDataStringsChunk = 9;
    }

    const char* ZMetaKey::GetString()
    {
        if (lValue != 1024 || !Animation::instance)
        {
            return nullptr;
        }

        return Animation::instance->GetMetaKeyDataStrings() + lStringOffset;
    }

    // Manager
    Manager::Manager()
    {
        Animation::instance = this;
        m_PlayUncompressed = false;
        m_Cache = nullptr;
        m_Animcount = 0;
        m_Headers = nullptr;
        m_iCrowdAnimCount = 0;
        m_pCrowdHeaders = nullptr;
        m_Data = nullptr;
        m_Pos = 0;
        m_OwnsBuffers = 0;
        m_SizeUncompressed = 0;
        m_StateSize = 0;
        m_HumanQuatSize = 0;
        m_QuatSize = 0;
        m_PoseSize = 0;

        ZHumanState::InitStaticData();
    }

    Manager::~Manager()
    {
        Clear();
        Animation::instance = nullptr;
    }

    char* Manager::GetAnimName(int id)
    {
        return const_cast<char*>(m_AnimNames.GetName(id, -1));
    }

    BoneID Manager::GetBoneID(const char* pszAnimName)
    {
        return m_BoneNames.GetId(pszAnimName, -1);
    }

    void* Manager::GetArray(void*& pBuffer, int lOffset)
    {
        auto* result = pBuffer;
        pBuffer = static_cast<void*>(static_cast<uint8_t*>(pBuffer) + lOffset);
        return result;
    }

    void Manager::GetInt32(void*& pBuffer, int32_t& result)
    {
        result = *static_cast<int32_t*>(pBuffer);
        pBuffer = static_cast<void*>(static_cast<uint8_t*>(pBuffer) + sizeof(int32_t));
    }

    Header* Manager::FromIndex(int lIndex)
    {
        if (lIndex < 0 || lIndex >= m_Animcount)
        {
            return nullptr;
        }

        return &m_Headers[lIndex];
    }

    int Manager::ToIndex(const Header* pHeader)
    {
        if (!m_Headers || !pHeader)
        {
            return -1;
        }

        return static_cast<int>(pHeader - m_Headers);
    }

    PoseID Manager::GetPoseID(const char* pszPoseName)
    {
        return m_PoseNames.GetId(pszPoseName, -1);
    }

    const char* Manager::GetMetaKeyDataStrings()
    {
        return m_pMetaKeyDataStrings;
    }

    bool Manager::GetPlayUncompressed()
    {
        return m_PlayUncompressed;
    }

    Manager* Manager::CreateFromDataBlock(void* pBlock, int lSize)
    {
        auto* memory = static_cast<Manager*>(ZUniMemory::Allocate(sizeof(Manager)));
        Manager* manager = znew_placement(memory);
        if (!manager)
        {
            return nullptr;
        }

        manager->LoadDataBlock(pBlock, lSize);
        return manager;
    }

    int Manager::Clear()
    {
        if (m_OwnsBuffers)
        {
            ZUniMemory::Delete(m_Headers);
            ZUniMemory::Delete(m_Data);
            ZUniMemory::Delete(m_BoneNames.m_Names);
            ZUniMemory::Delete(m_AnimNames.m_Names);
            ZUniMemory::Delete(m_PoseNames.m_Names);
        }

        if (m_Cache)
        {
            ZUniMemory::Delete(m_Cache);
            m_Cache = nullptr;
        }

        m_Cache = nullptr;
        m_Animcount = 0;
        m_Data = nullptr;
        m_Pos = 0;
        m_Headers = nullptr;
        m_BoneNames = {};
        m_AnimNames = {};
        m_Pos = {};
        m_OwnsBuffers = 0;

        return 0;
    }

    uint32_t Manager::GetMetaKeyDataLength(int lMetaKey)
    {
        return *reinterpret_cast<uint32_t*>(m_pMetaKeyData + (lMetaKey * 16));
    }

    ZMetaKey* Manager::GetMetaKeyData(int lMetaKey)
    {
        return reinterpret_cast<ZMetaKey*>(m_pMetaKeyData + (lMetaKey * 16) + 16);
    }

    ZMetaKey* Manager::GetMetaKeyAtFrame(int lMetaKey, uint32_t lFrame)
    {
        const uint32_t metaKeyDataLength = GetMetaKeyDataLength(lMetaKey);
        ZMetaKey* metaKeyData = GetMetaKeyData(lMetaKey);

        for (uint32_t i = 0; i < metaKeyDataLength; ++i)
        {
            if (static_cast<uint32_t>(metaKeyData[i].lFrame) == lFrame)
            {
                return &metaKeyData[i];
            }
        }

        return nullptr;
    }

    int32_t Manager::GetFrameFromMetaValue(int lMetaKey, uint32_t value, int32_t* pIndex)
    {
        const uint32_t metaKeyDataLength = GetMetaKeyDataLength(lMetaKey);
        ZMetaKey* metaKeyData = GetMetaKeyData(lMetaKey);
        int32_t index = pIndex ? *pIndex : 0;

        while (index < static_cast<int32_t>(metaKeyDataLength))
        {
            if (metaKeyData[index].lValue == value)
            {
                if (pIndex)
                {
                    *pIndex = index + 1;
                }

                return static_cast<int32_t>(metaKeyData[index].lFrame);
            }

            ++index;
        }

        if (pIndex)
        {
            *pIndex = -1;
        }

        return -1;
    }

    int32_t Manager::GetFrameFromMetaString(int lMetaKey, const char* pszString, uint32_t)
    {
        const uint32_t metaKeyDataLength = GetMetaKeyDataLength(lMetaKey);
        ZMetaKey* metaKeyData = GetMetaKeyData(lMetaKey);

        for (uint32_t i = 0; i < metaKeyDataLength; ++i)
        {
            const char* string = metaKeyData[i].GetString();
            if (string && !std::strcmp(string, pszString))
            {
                return static_cast<int32_t>(metaKeyData[i].lFrame);
            }
        }

        return -1;
    }

    void Manager::SetCompressionRatio(float ratio)
    {
        (void)ratio;
    }

    void Manager::SetCompressionRatioPC()
    {
    }

    int Manager::LoadDataBlock(void* pBlock, int lSize)
    {
        auto* block = static_cast<uint8_t*>(pBlock);
        (void)lSize;

        m_Animcount = *reinterpret_cast<int32_t*>(block + kAnimCountOffset);
        m_Pos = *reinterpret_cast<int32_t*>(block + kDataSizeOffset);
        m_BoneNames.m_Size = *reinterpret_cast<int32_t*>(block + kBoneNamesSizeOffset);
        m_AnimNames.m_Size = *reinterpret_cast<int32_t*>(block + kAnimNamesSizeOffset);
        m_PoseNames.m_Size = *reinterpret_cast<int32_t*>(block + kPoseNamesSizeOffset);
        m_BoneNames.m_Count = *reinterpret_cast<int32_t*>(block + kBoneNamesCountOffset);
        m_AnimNames.m_Count = *reinterpret_cast<int32_t*>(block + kAnimNamesCountOffset);
        m_PoseNames.m_Count = *reinterpret_cast<int32_t*>(block + kPoseNamesCountOffset);
        m_SizeUncompressed = *reinterpret_cast<int32_t*>(block + kUncompressedDataSizeOffset);
        m_StateSize = *reinterpret_cast<int32_t*>(block + kStateSizeOffset);
        m_HumanQuatSize = *reinterpret_cast<int32_t*>(block + kHumanQuatSizeOffset);
        m_QuatSize = *reinterpret_cast<int32_t*>(block + kQuatSizeOffset);
        m_PoseSize = *reinterpret_cast<int32_t*>(block + kPoseSizeOffset);

        auto* cursor = block + kHeaderArrayOffset;
        m_Headers = m_Animcount != 0
            ? reinterpret_cast<Header*>(cursor)
            : nullptr;
        cursor += m_Animcount * sizeof(Header);

        m_Data = m_Pos != 0 ? cursor : nullptr;
        cursor += m_Pos;

        m_BoneNames.m_Names = m_BoneNames.m_Size != 0
            ? reinterpret_cast<char*>(cursor)
            : nullptr;
        cursor += m_BoneNames.m_Size;

        m_AnimNames.m_Names = m_AnimNames.m_Size != 0
            ? reinterpret_cast<char*>(cursor)
            : nullptr;
        cursor += m_AnimNames.m_Size;

        m_PoseNames.m_Names = m_PoseNames.m_Size != 0
            ? reinterpret_cast<char*>(cursor)
            : nullptr;

        ZASSERT((reinterpret_cast<std::uintptr_t>(m_Data) & kPointerAlignmentMask) == 0);
        ZASSERT((reinterpret_cast<std::uintptr_t>(m_Headers) & kPointerAlignmentMask) == 0);

        m_OwnsBuffers = 0;
        if (m_Animcount != 0)
        {
            m_Headers[0].m_Name = m_AnimNames.m_Names;
            for (int i = 1; i < m_Animcount; ++i)
            {
                const char* previousName = m_Headers[i - 1].m_Name;
                while (*previousName)
                {
                    ++previousName;
                }

                m_Headers[i].m_Name = const_cast<char*>(previousName + 1);
            }
        }

        CHUNKFILE* packedAnims = g_pSysInterface->m_pEngineData->m_pPackedAnims;
        if (packedAnims)
        {
            CHUNKFILE* metaKeyData = packedAnims->FindChild(kMetaKeyDataChunk);
            CHUNKFILE* metaKeyDataStrings = packedAnims->FindChild(kMetaKeyDataStringsChunk);
            m_pMetaKeyData = metaKeyData ? static_cast<char*>(metaKeyData->Data()) : nullptr;
            m_pMetaKeyDataStrings = metaKeyDataStrings
                ? static_cast<char*>(metaKeyDataStrings->Data())
                : nullptr;
        }
        else
        {
            m_pMetaKeyData = nullptr;
            m_pMetaKeyDataStrings = nullptr;
        }

        m_Cache = nullptr;
        return 0;
    }


    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(Manager*, instance, 0x009AC998, nullptr);
}
