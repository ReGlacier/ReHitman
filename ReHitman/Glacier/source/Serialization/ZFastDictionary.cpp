#include <Glacier/ZUniMemory.h>
#include <Glacier/Serializer/ZDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZTokenTable_Serializerlib.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/ZInputStreamBase.h>
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

        uint32_t CountNodeWordBytes(const ZFastDictionary::cNode& node, bool includeTerminators, uint32_t depth)
        {
            uint32_t result = 0;

            if (node.m_Token != ZToken::Void)
                result = includeTerminators ? depth + 1 : depth;

            for (auto* pChild : node.m_Children)
            {
                if (!pChild)
                    continue;

                result += CountNodeWordBytes(*pChild, includeTerminators, depth + 1);
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

    ZToken ZDictionary::GetToken(const char* word)
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
                return entry.m_Token;
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
        return ZToken(static_cast<int32_t>(stringIndex));
    }

    ZDictionary_Serializerlib::~ZDictionary_Serializerlib() = default;

    ZDictionary_Serializerlib::ZDictionary_Serializerlib() = default;

    void ZDictionary_Serializerlib::SaveAsTokenTable(ZOutputStreamBase& pStream)
    {
        ZTokenTable_Serializerlib tokenTable(*this);
        tokenTable.Save(pStream);
    }

    void ZDictionary_Serializerlib::SaveAsPackedDictionary(ZOutputStreamBase& pStream)
    {
        ZPackedDictionary_Serializerlib packedDictionary(*this);
        packedDictionary.Save(pStream);
    }

    ZFastDictionary::cNode::cNode()
    {
        std::memset(&m_Children[0], 0x0, sizeof(m_Children));
        m_Token = ZToken::Void;
    }

    ZFastDictionary::cNode::~cNode()
    {
        for (auto*& pChild : m_Children)
        {
            if (!pChild)
                continue;

            pChild->~cNode();
            ZUniMemory::Free(pChild);
            pChild = nullptr;
        }
    }

    uint32_t ZFastDictionary::cNode::CountWordsLength(bool includeTerminators) const
    {
        return CountNodeWordBytes(*this, includeTerminators, 0);
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

        for (uint32_t i = 0; i < NUM_CHILD_NODES; ++i)
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

    int ZFastDictionary::cNode::Pack(ZPackedDictionary& dict, int& count)
    {
        int childIndex = count;

        for (int i = 0; i < NUM_CHILD_NODES; ++i)
        {
            if (m_Children[i])
                ++count;
        }

        for (int j = 0; j < NUM_CHILD_NODES; ++j)
        {
            if (m_Children[j])
            {
                dict.m_Letters[childIndex] = static_cast<char>(j);
                dict.m_From[childIndex] = count;
                dict.m_To[childIndex] = m_Children[j]->Pack(dict, count);
                dict.m_Tokens[childIndex] = m_Children[j]->m_Token;
                ++childIndex;
            }
        }

        return childIndex;
    }

    uint32_t ZFastDictionary::cNode::CountNodes() const
    {
        uint32_t iCount { 1 };
        
        for (const auto& pNode : m_Children)
        {
            if (pNode)
            {
                iCount += pNode->CountNodes();
            }
        }

        return iCount;
    }

    ZFastDictionary::~ZFastDictionary() = default;

    ZToken ZFastDictionary::GetToken(const char* word)
    {
        auto* pNode = FindOrAdd(word);
        if (pNode->m_Token == ZToken::Void)
        {
            pNode->m_Token = ++m_NextToken;
        }

        return ZToken { pNode->m_Token };
    }

    ZFastDictionary::ZFastDictionary()
    {
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
    
    ZFastDictionary::cNode* ZFastDictionary::FindOrAdd(const char* pToken)
    {
        if (!pToken)
        {
            return &m_Root;
        }

        cNode* pNode = &m_Root;
        const char* pWord = pToken;
        int32_t index = 0;

        for (;;)
        {
            if (*pWord == '\0')
                return pNode;

            index = static_cast<uint8_t>(*pWord++) & 0x7F;

            if (!pNode->m_Children[index])
                break;

            pNode = pNode->m_Children[index];
        }

        while (true)
        {
            auto* pNewNode = ZUniMemory::New<cNode>();

            pNode->m_Children[index] = pNewNode;
            pNode = pNewNode;

            if (*pWord == '\0')
                break;

            index = static_cast<uint8_t>(*pWord++) & 0x7F;
        }

        return pNode;
    }

    uint32_t ZFastDictionary::CountNodes() const
    {
        return m_Root.CountNodes();
    }

    ZPackedDictionary::~ZPackedDictionary()
    {
        Cleanup();
    }

    ZPackedDictionary::ZPackedDictionary()
        : m_Size(0)
        , m_Letters(nullptr)
        , m_From(nullptr)
        , m_To(nullptr)
        , m_Tokens(nullptr)
    {
    }

    void ZPackedDictionary::Setup()
    {
        m_Letters = static_cast<char*>(ZUniMemory::Allocate(m_Size));
        m_From = static_cast<uint32_t*>(ZUniMemory::Allocate(m_Size * sizeof(uint32_t)));
        m_To = static_cast<uint32_t*>(ZUniMemory::Allocate(m_Size * sizeof(uint32_t)));
        m_Tokens = static_cast<ZToken*>(ZUniMemory::Allocate(m_Size * sizeof(ZToken)));

        for (uint32_t i = 0; i < m_Size; ++i)
            new (&m_Tokens[i]) ZToken();
    }

    void ZPackedDictionary::Cleanup()
    {
        ZUniMemory::Free(m_Letters);
        ZUniMemory::Free(m_From);
        ZUniMemory::Free(m_To);
        ZUniMemory::Free(m_Tokens);
        m_Letters = nullptr;
        m_From = nullptr;
        m_To = nullptr;
        m_Tokens = nullptr;
    }

    uint32_t ZPackedDictionary::CalculatePackedSize(const TDynamicArray<ZDictionary::ZTokenizedString>& tokenTable,
                                                     int low, int high, int depth)
    {
        uint32_t nodeCount = 0;
        int current = low;

        while (current < high)
        {
            int groupStart = current;
            char c = static_cast<const char*>(tokenTable[current].m_String)[depth];
            ++current;

            while (current < high)
            {
                if (static_cast<const char*>(tokenTable[current].m_String)[depth] != c)
                    break;
                ++current;
            }

            ++nodeCount;

            const char* firstStr = static_cast<const char*>(tokenTable[groupStart].m_String);
            if (!firstStr[depth + 1] && ++groupStart == current)
                continue;

            nodeCount += CalculatePackedSize(tokenTable, groupStart, current, depth + 1);
        }

        return nodeCount;
    }

    uint32_t ZPackedDictionary::PackRecursive(const TDynamicArray<ZDictionary::ZTokenizedString>& tokenTable,
                                               int& count, int low, int high, int depth)
    {
        int savedCount = count;
        int current = low;

        while (current < high)
        {
            int groupStart = current;
            char c = static_cast<const char*>(tokenTable[current].m_String)[depth];
            ++current;

            while (current < high)
            {
                if (static_cast<const char*>(tokenTable[current].m_String)[depth] != c)
                    break;
                ++current;
            }

            const char* firstStr = static_cast<const char*>(tokenTable[groupStart].m_String);
            if (firstStr[depth + 1])
            {
                m_Tokens[count] = ZToken::Unknown;
            }
            else
            {
                m_Tokens[count] = tokenTable[groupStart].m_Token;
                ++groupStart;
            }

            m_From[count] = static_cast<uint32_t>(groupStart);
            m_To[count] = static_cast<uint32_t>(current);
            m_Letters[count] = c;
            ++count;
        }

        const int levelEnd = count;

        for (int i = savedCount; i < levelEnd; ++i)
        {
            uint32_t pageStart = m_From[i];
            uint32_t pageEnd = m_To[i];

            if (pageStart < pageEnd)
            {
                m_From[i] = static_cast<uint32_t>(count);
                m_To[i] = PackRecursive(tokenTable, count,
                                        static_cast<int>(pageStart),
                                        static_cast<int>(pageEnd),
                                        depth + 1);
            }
        }

        return static_cast<uint32_t>(levelEnd);
    }

    ZPackedDictionary::ZPackedDictionary(ZDictionary& sDict)
    {
        int tokenCount = static_cast<int>(sDict.m_TokenTable.GetSize());
        m_Size = CalculatePackedSize(sDict.m_TokenTable, 0, tokenCount, 0) + 1;
        Setup();

        int count = 1;
        m_From[0] = 1;
        m_To[0] = PackRecursive(sDict.m_TokenTable, count, 0, tokenCount, 0);
        m_Tokens[0] = ZToken::Unknown;
    }

    ZPackedDictionary::ZPackedDictionary(ZFastDictionary& sDict)
    {
        m_Size = sDict.CountNodes();
        Setup();

        int count = 1;
        m_From[0] = 1;
        m_To[0] = static_cast<uint32_t>(sDict.m_Root.Pack(*this, count));
        m_Tokens[0] = sDict.m_Root.m_Token;
    }

    ZPackedDictionary_Serializerlib::~ZPackedDictionary_Serializerlib() = default;

    ZPackedDictionary_Serializerlib::ZPackedDictionary_Serializerlib() = default;

    ZPackedDictionary_Serializerlib::ZPackedDictionary_Serializerlib(ZDictionary& sDict)
        : ZPackedDictionary(sDict)
    {
    }

    ZPackedDictionary_Serializerlib::ZPackedDictionary_Serializerlib(ZFastDictionary& sDict)
        : ZPackedDictionary(sDict)
    {
    }

    void ZPackedDictionary_Serializerlib::Save(ZOutputStreamBase& pStream)
    {
        pStream.WriteWithEndianness<uint32_t>(m_Size);
        pStream.Write(m_Letters, m_Size);
        pStream.WriteWithEndianness(m_From, m_Size);
        pStream.WriteWithEndianness(m_To, m_Size);

        for (int i = 0; i < m_Size; ++i)
        {
            pStream.WriteWithEndianness(static_cast<uint32_t>(m_Tokens[i]));
        }
    }

    void ZPackedDictionary_Serializerlib::Load(ZInputStreamBase& stream)
    {
        Cleanup();

        m_Size = stream.GetAndChangeEndiannessIfRequired<uint32_t>();
        Setup();

        stream.Read(m_Letters, m_Size);
        stream.GetAndChangeEndiannessIfRequired(m_From, m_Size);
        stream.GetAndChangeEndiannessIfRequired(m_To, m_Size);

        for (int i = 0; i < m_Size; ++i)
        {
            m_Tokens[i] = ZToken(stream.GetAndChangeEndiannessIfRequired<uint32_t>());
        }
    }

    ZToken ZPackedDictionary::GetToken(const char* word)
    {
        if (!m_Size)
        {
            return ZToken::Void;
        }

        uint32_t node = 0;
        const char* pRead = word;

        for (;;)
        {
            if (!*pRead)
            {
                return m_Tokens[node];
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
                return ZToken::Unknown;
            }
        }
    }
}
