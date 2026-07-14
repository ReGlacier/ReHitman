#include <Glacier/Serializer/ZDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/ZPackedDictionary.h>
#include <Glacier/Serializer/ZFastDictionary.h>
#include <Glacier/Serializer/ZDictionary.h>
#include <Glacier/Serializer/ZTokenTable.h>

#include <cstdint>
#include <cstring>


namespace Glacier
{
    namespace
    {
        int32_t GetPackedDictionaryIndex(const char*& pWord)
        {
            return static_cast<uint8_t>(*pWord++) & 0x7F;
        }

        uint32_t CountPackedNodeBytes(const ZFastDictionary::cNode& node, bool includeTerminators, uint32_t depth)
        {
            uint32_t result = 0;
            bool hasWrittenWord = node.m_Token != ZToken::Void;

            if (hasWrittenWord && includeTerminators)
                ++result;

            for (auto* pChild : node.m_Children)
            {
                if (!pChild)
                    continue;

                if (hasWrittenWord)
                    result += depth;

                result += 1 + CountPackedNodeBytes(*pChild, includeTerminators, depth + 1);
                hasWrittenWord = true;
            }

            return result;
        }
    }

    IDictionary::~IDictionary() = default;

    ZDictionary::ZTokenizedString::ZTokenizedString() = default;

    ZDictionary::ZTokenizedString::ZTokenizedString(const ZTokenizedString& copy) = default;

    ZDictionary::ZTokenizedString& ZDictionary::ZTokenizedString::operator=(const ZTokenizedString& copy) = default;

    ZDictionary::~ZDictionary() = default;

    ZDictionary::ZDictionary()
        : m_StringTable(ZRTString())
        , m_TokenTable(ZTokenizedString())
    {
    }

    ZToken* ZDictionary::GetToken(ZToken* result, const char* word)
    {
        const char* pWord = word ? word : "";
        uint32_t low = 0;
        uint32_t high = m_TokenTable.GetSize();

        while (low < high)
        {
            const uint32_t mid = (low + high) >> 1;
            ZTokenizedString& entry = m_TokenTable[mid];

            if (entry.m_String == pWord)
            {
                *result = entry.m_Token;
                return result;
            }

            if (entry.m_String < pWord)
                low = mid + 1;
            else
                high = mid;
        }

        const uint32_t stringIndex = m_StringTable.GetSize();
        m_StringTable[stringIndex] = pWord;

        m_TokenTable[stringIndex];

        for (uint32_t i = stringIndex; low < i; --i)
            m_TokenTable[i] = m_TokenTable[i - 1];

        m_TokenTable[low].m_String = pWord;
        m_TokenTable[low].m_Token = ZToken(static_cast<int32_t>(stringIndex));
        *result = ZToken(static_cast<int32_t>(stringIndex));
        return result;
    }

    ZDictionary_Serializerlib::~ZDictionary_Serializerlib() = default;

    ZDictionary_Serializerlib::ZDictionary_Serializerlib() = default;

    void ZDictionary_Serializerlib::SaveAsTokenTable(ZOutputStreamBase* pStream)
    {
        ZTokenTable_Serializerlib tokenTable(*this);
        tokenTable.Save(pStream);
    }

    void ZDictionary_Serializerlib::SaveAsPackedDictionary(ZOutputStreamBase* pStream)
    {
        ZPackedDictionary_Serializerlib packedDictionary(*this);
        packedDictionary.Save(pStream);
    }

    uint32_t ZFastDictionary::cNode::CountWordsLength(bool includeTerminators) const
    {
        return CountPackedNodeBytes(*this, includeTerminators, 0);
    }

    ZToken ZFastDictionary::cNode::GetLargestToken(ZToken largest) const
    {
        for (auto* pChild : m_Children)
        {
            if (pChild)
                largest = pChild->GetLargestToken(largest);
        }

        return largest < m_Token ? m_Token : largest;
    }

    char* ZFastDictionary::cNode::Unpack(char* pWrite, char* pWord, uint32_t lDepth, const char** pToken2Name) const
    {
        bool hasWord = m_Token != ZToken::Void;

        if (hasWord)
        {
            pToken2Name[static_cast<int32_t>(m_Token)] = pWord;
            *pWrite++ = '\0';
        }

        for (uint32_t i = 0; i < 128; ++i)
        {
            auto* pChild = m_Children[i];
            if (!pChild)
                continue;

            if (hasWord)
            {
                std::memcpy(pWrite, pWord, lDepth);
                pWord = pWrite;
                pWrite += lDepth;
            }

            *pWrite = static_cast<char>(i);
            pWrite = pChild->Unpack(pWrite + 1, pWord, lDepth + 1, pToken2Name);
            hasWord = true;
        }

        return pWrite;
    }

    uint32_t ZFastDictionary::CountWordsLength(bool includeTerminators) const
    {
        return m_Root.CountWordsLength(includeTerminators);
    }

    ZToken ZFastDictionary::GetLargestToken() const
    {
        return m_Root.GetLargestToken(ZToken::Void);
    }

    void ZFastDictionary::Unpack(char* pWords, const char** pToken2Name) const
    {
        m_Root.Unpack(pWords, pWords, 0, pToken2Name);
    }

    ZPackedDictionary::~ZPackedDictionary() = default;

    ZPackedDictionary::ZPackedDictionary()
        : m_Size(0)
        , m_Letters(nullptr)
        , m_From(nullptr)
        , m_To(nullptr)
        , m_Tokens(nullptr)
    {
    }

    ZPackedDictionary_Serializerlib::~ZPackedDictionary_Serializerlib() = default;

    ZPackedDictionary_Serializerlib::ZPackedDictionary_Serializerlib() = default;

    ZPackedDictionary_Serializerlib::ZPackedDictionary_Serializerlib(ZDictionary&)
        : ZPackedDictionary()
    {
    }

    void ZPackedDictionary_Serializerlib::Save(ZOutputStreamBase* pStream)
    {
        pStream->Write(m_Size);

        if (!m_Size)
            return;

        pStream->Write(m_Letters, m_Size);
        pStream->WriteRaw(reinterpret_cast<char*>(m_From), sizeof(uint32_t) * m_Size);
        pStream->WriteRaw(reinterpret_cast<char*>(m_To), sizeof(uint32_t) * m_Size);
        pStream->WriteRaw(reinterpret_cast<char*>(m_Tokens), sizeof(ZToken) * m_Size);
    }

    ZToken* ZPackedDictionary::GetToken(ZToken* result, const char* word)
    {
        if (!m_Size)
        {
            *result = ZToken::Void;
            return result;
        }

        uint32_t node = 0;
        const char* pRead = word;

        for (;;)
        {
            if (!*pRead)
            {
                *result = m_Tokens[node];
                return result;
            }

            const int32_t index = GetPackedDictionaryIndex(pRead);
            uint32_t from = m_From[node];
            uint32_t to = m_To[node];

            while (from < to)
            {
                node = (from + to) >> 1;
                const int32_t letter = static_cast<uint8_t>(m_Letters[node]) & 0x7F;

                if (index >= letter)
                {
                    if (letter >= index)
                        break;

                    from = node + 1;
                }
                else
                {
                    to = node;
                }
            }

            if (from >= to)
            {
                *result = ZToken::Unknown;
                return result;
            }
        }
    }
}
