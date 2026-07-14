#include <Glacier/Serializer/ZTokenTable.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/IDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZFastDictionary.h>
#include <Glacier/Serializer/ZDictionary.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>


namespace Glacier
{
    IDictionary_Serializerlib::~IDictionary_Serializerlib() = default;

    ZOutputStreamBase::~ZOutputStreamBase() = default;

    ZTokenTable::~ZTokenTable()
    {
        if (m_Words)
        {
            ZUniMemory::Free(m_Words);
            m_Words = nullptr;
        }

        if (m_Token2Name)
        {
            ZUniMemory::Free(m_Token2Name);
            m_Token2Name = nullptr;
        }
    }

    ZTokenTable::ZTokenTable()
        : m_LargestToken(ZToken())
        , m_Words{nullptr}
        , m_Token2Name{nullptr}
    {
    }


    ZTokenTable::ZTokenTable(ZFastDictionary& dict)
        : m_LargestToken(ZToken())
        , m_Words{nullptr}
        , m_Token2Name{nullptr}
    {
        uint32_t wordsSize = dict.CountWordsLength(true);
        m_Words = static_cast<char*>(ZUniMemory::Allocate(wordsSize));
        m_LargestToken = dict.GetLargestToken();
        m_Token2Name = static_cast<const char**>(ZUniMemory::Allocate(sizeof(const char*) * (static_cast<int32_t>(m_LargestToken) + 1)));
        dict.Unpack(m_Words, m_Token2Name);
    }

    ZTokenTable::ZTokenTable(ZDictionary& dict)
        : m_LargestToken(ZToken())
        , m_Words{nullptr}
        , m_Token2Name{nullptr}
    {
        m_LargestToken = ZToken(static_cast<int32_t>(dict.m_StringTable.GetSize()) - 1);

        uint32_t wordsSize = 0;
        for (int32_t i = 0; i <= static_cast<int32_t>(m_LargestToken); ++i)
        {
            const char* pWord = dict.m_StringTable[static_cast<uint32_t>(i)].c_str();
            wordsSize += static_cast<uint32_t>(std::strlen(pWord)) + 1;
        }

        m_Words = static_cast<char*>(ZUniMemory::Allocate(wordsSize));
        m_Token2Name = static_cast<const char**>(ZUniMemory::Allocate(sizeof(const char*) * (static_cast<int32_t>(m_LargestToken) + 1)));

        char* pWrite = m_Words;
        for (int32_t i = 0; i <= static_cast<int32_t>(m_LargestToken); ++i)
        {
            const char* pWord = dict.m_StringTable[static_cast<uint32_t>(i)].c_str();
            uint32_t length = static_cast<uint32_t>(std::strlen(pWord)) + 1;
            m_Token2Name[i] = pWrite;
            std::memcpy(pWrite, pWord, length);
            pWrite += length;
        }
    }

    bool ZTokenTable::IsValidToken(ZToken token) const
    {
        return token <= m_LargestToken && token > ZToken::Void;
    }

    const char* ZTokenTable::GetWord(ZToken token) const
    {
        // common/basic/dictionary.h
        ZASSERT(IsValidToken(token));
        return m_Token2Name[token];
    }

    ZTokenTable_Serializerlib::~ZTokenTable_Serializerlib() = default;

    ZTokenTable_Serializerlib::ZTokenTable_Serializerlib() = default;

    ZTokenTable_Serializerlib::ZTokenTable_Serializerlib(ZDictionary& dict)
        : ZTokenTable(dict)
    {
    }

    ZTokenTable_Serializerlib::ZTokenTable_Serializerlib(ZFastDictionary& dict)
        : ZTokenTable(dict)
    {
    }

    void ZTokenTable_Serializerlib::Save(ZOutputStreamBase* pStream)
    {
        const uint32_t largestToken = static_cast<uint32_t>(static_cast<int32_t>(m_LargestToken));
        pStream->Write(largestToken);

        uint32_t wordsSize = 0;
        for (ZToken token(0); token <= m_LargestToken; ++token)
        {
            const char* pWord = m_Token2Name[static_cast<int32_t>(token)];
            wordsSize += static_cast<uint32_t>(std::strlen(pWord)) + 1;
        }

        pStream->Write(wordsSize);

        for (ZToken token(0); token <= m_LargestToken; ++token)
        {
            const char* pWord = m_Token2Name[static_cast<int32_t>(token)];
            const uint32_t length = static_cast<uint32_t>(std::strlen(pWord)) + 1;
            pStream->Write(pWord, length);
        }
    }
}
