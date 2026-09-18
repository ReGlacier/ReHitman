#include <Glacier/Serializer/ZTokenTable_Serializerlib.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/ZInputStreamBase.h>
#include <Glacier/Serializer/ZFastDictionary.h>
#include <Glacier/Serializer/ZDictionary.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    ZTokenTable_Serializerlib::~ZTokenTable_Serializerlib() = default;
    
    ZTokenTable_Serializerlib::ZTokenTable_Serializerlib()
        : ZTokenTable()
    {}

    ZTokenTable_Serializerlib::ZTokenTable_Serializerlib(ZFastDictionary& dict)
        : ZTokenTable(dict)
    {}

    ZTokenTable_Serializerlib::ZTokenTable_Serializerlib(ZDictionary& dict)
        : ZTokenTable(dict)
    {}
        
    void ZTokenTable_Serializerlib::Save(ZOutputStreamBase& stream)
    {
        stream.Write(static_cast<int32_t>(m_LargestToken));

        ZToken currentToken { 0 };
        uint32_t totalLen { 0 };

        while (currentToken <= m_LargestToken)
        {
            totalLen += strlen(m_Token2Name[static_cast<int32_t>(currentToken)]) + 1;
            ++currentToken;
        }

        stream.Write<uint32_t>(totalLen);

        currentToken = 0;
        while (true)
        {
            if (currentToken > m_LargestToken)
                break;

            int iLen = strlen(m_Token2Name[static_cast<int32_t>(currentToken)]) + 1;
            stream.Write(m_Token2Name[static_cast<int32_t>(currentToken)], sizeof(char) * iLen);
            ++currentToken;
        }
    }

    void ZTokenTable_Serializerlib::Load(ZInputStreamBase& stream)
    {
        m_LargestToken = stream.GetAndChangeEndiannessIfRequired<uint32_t>();

        if (m_Token2Name)
        {
            ZUniMemory::Free(m_Token2Name);
            m_Token2Name = nullptr;
        }

        m_Token2Name = (const char**)ZUniMemory::Allocate((static_cast<int32_t>(m_LargestToken) + 1) * sizeof(char*));
        const uint32_t iTotalLength = stream.GetAndChangeEndiannessIfRequired<uint32_t>();

        if (m_Words)
        {
            ZUniMemory::Free(m_Words);
            m_Words = nullptr;
        }

        m_Words = (char*)ZUniMemory::Allocate(iTotalLength * sizeof(char));
        stream.ReadRaw(m_Words, iTotalLength);

        ZToken currenToken { 0 };
        char* pCurrentWord = m_Words;
        while (true)
        {
            if (currenToken > m_LargestToken)
                break;

            m_Token2Name[static_cast<int32_t>(currenToken)] = pCurrentWord;
            pCurrentWord += strlen(pCurrentWord) + 1;

            ++currenToken;
        }
    }
}
