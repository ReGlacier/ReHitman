#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/Filesystem/IBuffer.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace 
    {
        const char* NameToString(uint32_t dwName)
        {
            static thread_local char szName[5];
    
            char* pOut = szName;
            uint32_t v1 = dwName;

            while (v1 != 0)
            {
                uint8_t c = static_cast<uint8_t>((v1 >> 24) & 0xFF);

                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
                {
                    *pOut = static_cast<char>(c);
                }
                else
                {
                    *pOut = '.';
                }

                ++pOut;
                v1 <<= 8;
            }

            *pOut = '\0';
            return szName;
        }
        
        const float* GetFloatArrayPointer(const ZRPropertyReader* pReader)
        {
            ZASSERT(pReader && pReader->m_pProperty);

            if (pReader->m_pProperty->lSize == 1)
            {
                return reinterpret_cast<const float*>(&pReader->m_pProperty->lData);
            }

            return static_cast<const float*>(
                const_cast<IBuffer*>(pReader->m_pBuffer)->GetData(pReader->m_pProperty->lData)
            );
        }

        void DumpProperty(const ZRPropertyReader& reader, uint32_t nIndent, const char* pszHeader)
        {
            const ZRPropertyReader::SProperty* pProp = reader.m_pProperty;
            if (!pProp)
                return;

            if (pszHeader && *pszHeader)
            {
                printf("%*s=== %s ===\n", static_cast<int>(nIndent), "", pszHeader);
            }

            const char* pszTypeName = "Unknown";
            switch (pProp->lType)
            {
                case ZRPropertyReader::PT_FLOAT:  pszTypeName = "Float";  break;
                case ZRPropertyReader::PT_CHAR:   pszTypeName = "Char";   break;
                case ZRPropertyReader::PT_UINT32: pszTypeName = "UInt32"; break;
                case ZRPropertyReader::PT_LIST:   pszTypeName = "List";   break;
            }

            const char* pszName = NameToString(pProp->lName);

            printf("%*sName:%-4s Type:%-10s Size:%-4u\n", 
                static_cast<int>(nIndent), "", 
                pszName, 
                pszTypeName, 
                pProp->lSize);

            switch (pProp->lType)
            {
                case ZRPropertyReader::PT_FLOAT:
                {
                    const float* pFloats = GetFloatArrayPointer(&reader);
                    if (pFloats)
                    {
                        for (uint32_t i = 0; i < pProp->lSize; ++i)
                        {
                            printf("%*s  [%2u] %.4f\n", static_cast<int>(nIndent), "", i, pFloats[i]);
                        }
                    }
                    break;
                }

                case ZRPropertyReader::PT_CHAR:
                {
                    const char* pszStr = nullptr;
                    if (reader.m_pBuffer)
                    {
                        pszStr = static_cast<const char*>(
                            const_cast<IBuffer*>(reader.m_pBuffer)->GetData(pProp->lData)
                        );
                    }
                    printf("%*s  String: %s\n", static_cast<int>(nIndent), "", pszStr ? pszStr : "(null)");
                    break;
                }

                case ZRPropertyReader::PT_UINT32:
                {
                    const uint32_t* pInts = nullptr;
                    if (pProp->lSize == 1)
                    {
                        pInts = &pProp->lData;
                    }
                    else if (reader.m_pBuffer)
                    {
                        pInts = static_cast<const uint32_t*>(
                            const_cast<IBuffer*>(reader.m_pBuffer)->GetData(pProp->lData)
                        );
                    }

                    if (pInts)
                    {
                        for (uint32_t i = 0; i < pProp->lSize; ++i)
                        {
                            printf("%*s  [%2u] %u\n", static_cast<int>(nIndent), "", i, pInts[i]);
                        }
                    }
                    break;
                }

                case ZRPropertyReader::PT_LIST:
                {
                    if (reader.m_pBuffer)
                    {
                        auto pChildElements = static_cast<const ZRPropertyReader::SProperty*>(
                            const_cast<IBuffer*>(reader.m_pBuffer)->GetData(pProp->lData)
                        );

                        for (uint32_t i = 0; i < pProp->lSize; ++i)
                        {
                            ZRPropertyReader childReader;
                            childReader.m_pBuffer = reader.m_pBuffer;
                            childReader.m_pProperty = const_cast<ZRPropertyReader::SProperty*>(&pChildElements[i]);

                            DumpProperty(childReader, nIndent + 2, nullptr);
                        }
                    }
                    break;
                }

                default:
                    ZASSERT(false);
                    break;
            }
        }
    }
    
    ZRPropertyReader::ZRPropertyReader() = default;
    
    void ZRPropertyReader::Dump(int nIndent, const char* pszHeader) const
    {
        DumpProperty(*this, static_cast<uint32_t>(nIndent), pszHeader);
    }

    void ZRPropertyReader::GetNamedListElement(uint32_t lName, ZRPropertyReader& outElement) const
    {
        ZASSERT(m_pProperty && m_pProperty->lType == PT_LIST);

        auto pElements = static_cast<const SProperty*>(const_cast<IBuffer*>(m_pBuffer)->GetData(m_pProperty->lData));
        uint32_t nCount = m_pProperty->lSize;

        for (uint32_t i = 0; i < nCount; ++i)
        {
            if (pElements[i].lName == lName)
            {
                outElement.m_pBuffer = m_pBuffer;
                outElement.m_pProperty = const_cast<SProperty*>(&pElements[i]);
                return;
            }
        }

        // Not found
        Dump(0, " <GetNamedListElement> ");
        ZASSERT(false);
    }

    bool ZRPropertyReader::TryGetNamedListElement(uint32_t lName, ZRPropertyReader& outElement) const
    {
        ZASSERT(m_pProperty && m_pProperty->lType == PT_LIST);

        auto pElements = static_cast<const SProperty*>(const_cast<IBuffer*>(m_pBuffer)->GetData(m_pProperty->lData));
        uint32_t nCount = m_pProperty->lSize;

        for (uint32_t i = 0; i < nCount; ++i)
        {
            if (pElements[i].lName == lName)
            {
                outElement.m_pBuffer = m_pBuffer;
                outElement.m_pProperty = const_cast<SProperty*>(&pElements[i]);
                return true;
            }
        }

        return false;
    }
}