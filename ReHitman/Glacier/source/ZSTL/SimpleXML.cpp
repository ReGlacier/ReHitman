#include <Glacier/ZSTL/SimpleXML.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        void skip(char** p, char* pEnd)
        {
            if (p == nullptr || *p == nullptr) 
                return;

            while (*p < pEnd)
            {
                char c = **p;

                if (c > ' ' && c != '.' && c != '-' && c != '_' && c != ':')
                {
                    break;
                }
                
                (*p)++;
            }

            if (*p == pEnd)
            {
                *p = nullptr;
            }
        }

        char* getsym(char** p, char* pEnd)
        {
            if (p == nullptr || *p == nullptr || *p >= pEnd) 
            {
                return nullptr;
            }

            char* pStartSymbol = *p;

            char firstChar = **p;
            (*p)++;

            if (firstChar == '"')
            {
                while (*p < pEnd)
                {
                    if (**p == '"')
                    {
                        break;
                    }
                    (*p)++;
                }
            }
            else
            {
                while (*p < pEnd)
                {
                    char c = **p;
                    
                    if (!isalnum(static_cast<unsigned char>(c)))
                    {
                        if (c != '.' && c != '-' && c != '_' && c != ':')
                        {
                            break;
                        }
                    }
                    (*p)++;
                }
            }

            return pStartSymbol;
        }
    }

    void SimpleXML::startElement(const char* name, const char** attrs)
    {
        // Empty impl by default
        std::ignore = name;
        std::ignore = attrs;
    }
    
    void SimpleXML::endElement(const char* name)
    {
        // Empty impl by default
        std::ignore = name;
    }

    SimpleXML::SimpleXML()
        : m_pStart(nullptr)
        , m_pEnd(nullptr)
        , m_p(nullptr)
        , m_Len(0)
        , m_pTag(nullptr)
        , m_nAttributes(0)
    {
        m_Attributes[0] = nullptr;
    }

    int SimpleXML::XML_Parse(char* buffer, int length, int isFinal)
    {
        m_pStart = buffer;
        m_p = buffer;
        m_pEnd = buffer + length;

        while (true)
        {
            int element = getElement();

            if (element >= XML_ERROR)
            {
                return (element != XML_ERROR);
            }

            if (element == XML_START || element == XML_STARTEND)
            {
                startElement(m_pTag, const_cast<const char**>(m_Attributes));
            }

            if (element == XML_END || element == XML_STARTEND)
            {
                endElement(m_pTag);
            }
        }
    }

    const char* SimpleXML::GetAttr(const char** attrs, const char* name)
    {
        if (attrs == nullptr || name == nullptr)
        {
            return nullptr;
        }

        for (int i = 0; attrs[i] != nullptr; i += 2)
        {
            if (strcmp(attrs[i], name) == 0)
            {
                return attrs[i + 1];
            }
        }

        return nullptr;
    }

    int SimpleXML::getElement()
    {
        m_nAttributes = 0;
        m_Attributes[0] = nullptr;
        m_pTag = nullptr;

        skip(&m_p, m_pEnd);

        if (m_p == nullptr || m_p >= m_pEnd)
        {
            return XML_ENDOFDATA; 
        }

        if (m_p + 3 < m_pEnd && m_p[0] == '<' && m_p[1] == '!' && m_p[2] == '-' && m_p[3] == '-')
        {
            char* i = m_p + 4;
            while (i < m_pEnd)
            {
                if (i >= m_pStart + 2 && i[0] == '>' && i[-1] == '-' && i[-2] == '-')
                {
                    break;
                }
                ++i;
            }

            if (i >= m_pEnd)
            {
                return printError("Missing end of comment");
            }
            
            m_p = i + 1;
            return XML_COMMENT;
        }

        if (*m_p == '<')
        {
            m_p++;

            if (*m_p == '/')
            {
                m_p++;
                m_pTag = getsym(&m_p, m_pEnd);

                if (m_p < m_pEnd && *m_p == '>')
                {
                    *m_p = '\0';
                    m_p++;
                    return XML_END;
                }
                return printError("Missing '>'");
            }

            m_pTag = getsym(&m_p, m_pEnd);
            if (m_pTag == nullptr)
            {
                return printError("EOF in symbol");
            }

            char nextChar = *m_p;
            *m_p = '\0';
            m_p++;

            if (nextChar == '>')
            {
                return XML_START;
            }

            if (nextChar == '/')
            {
                if (m_p < m_pEnd && *m_p == '>')
                {
                    m_p++;
                    return XML_STARTEND;
                }
            }
            else
            {
                while (true)
                {
                    skip(&m_p, m_pEnd);
                    
                    if (m_p >= m_pEnd || !isalpha(static_cast<unsigned char>(*m_p)))
                    {
                        break;
                    }

                    char* attrName = getsym(&m_p, m_pEnd);
                    skip(&m_p, m_pEnd);

                    if (m_p >= m_pEnd || *m_p != '=')
                    {
                        return printError("Missing '='");
                    }

                    *m_p = '\0';
                    m_p++;

                    char* attrValue = getsym(&m_p, m_pEnd);
                    if (m_p >= m_pEnd)
                    {
                        return printError("Attribute value error");
                    }

                    *m_p = '\0';
                    m_p++;

                    if (attrName && attrValue)
                    {
                        if (m_nAttributes >= NATTRIBUTES - 2)
                        {
                            return printError("Attribute overflow");
                        }

                        m_Attributes[m_nAttributes++] = attrName;
                        m_Attributes[m_nAttributes++] = attrValue;
                        m_Attributes[m_nAttributes] = nullptr;
                    }
                }

                int returnType = XML_START;
                
                if (m_p < m_pEnd && *m_p == '/')
                {
                    m_p++;
                    returnType = XML_STARTEND; // 2
                }

                if (m_p < m_pEnd && *m_p == '>')
                {
                    m_p++;
                    return returnType;
                }
            }

            return printError("Missing '>'");
        }

        return printError("Invalid XML format");
    }

    int SimpleXML::printError(const char* psErrorMessage)
    {
        printf("SimpleXML Error: %s (%.30s)\n", psErrorMessage, m_p);
        ZASSERT(false);
        return XML_ERROR;
    }
}
