#include <Glacier/Serializer/ZTokenStream.h>
#include <Glacier/ZUniAssert.h>
#include <cstring>
#include <cstdio>


namespace Glacier
{
    ZTokenStream::ZTokenStream(const char* psData)
        : m_szData(psData)
    {
        kaTokens[0] = ParseToken();
    }

    ZTokenStream::~ZTokenStream() = default;

    ZTokenStream::EToken ZTokenStream::Swallow()
    {
        auto eType = kaTokens[0].eType;
        kaTokens[0] = ParseToken();
        return eType;
    }

    bool ZTokenStream::Swallow(ZTokenStream::EToken eToken, float& fValue)
    {
        if (eToken == Peek())
        {
            fValue = kaTokens[0].fVal;
            Swallow();
            return true;
        }

        return false;
    }

    bool ZTokenStream::Swallow(ZTokenStream::EToken eToken, char* pStr)
    {
        if (eToken == Peek() || eToken == eSYMBOL && Peek() == eNUMBER)
        {
            strcpy(pStr, kaTokens[0].acVal);
            Swallow();
            return true;
        }

        printf("Action --> Error parsing action binds, expected token nr %i", eToken);
        printf("Action --> got (blank for end of string): %.50s", m_szData);
        ZASSERT(false);
        return false;
    }

    bool ZTokenStream::Swallow(ZTokenStream::EToken eToken)
    {
        if (eToken == Peek())
        {
            Swallow();
            return true;
        }
        
        printf("Action --> Error parsing action binds, expected token nr %i", eToken);
        printf("Action --> got (blank for end of string): %.50s", m_szData);
        ZASSERT(false);
        return false;
    }

    ZTokenStream::EToken ZTokenStream::Peek()
    {
        return kaTokens[0].eType;
    }
        
    ZTokenStream::ZTokenData ZTokenStream::ParseToken()
    {
        ZTokenData token;
        token.eType = eINVALID;
        token.fVal = 0.0f;
        token.acVal[0] = '\0';

        while (*m_szData)
        {
            // Skip space, new line, new tab, return carete
            while (*m_szData == ' ' || *m_szData == '\t' || *m_szData == '\n' || *m_szData == '\r')
            {
                ++m_szData;
            }

            // Skip comments (lmao, it has comments?!)
            if (m_szData[0] == '/' && m_szData[1] == '/')
            {
                while (*m_szData && *m_szData != '\n' && *m_szData != '\r')
                {
                    ++m_szData;
                }
            }
            else
            {
                // Stop iteration loop
                break;
            }
        }

        // End of line
        if (*m_szData == '\0')
        {
            token.eType = eEND;
            return token;
        }

        // Operators
        char c = *m_szData;
        switch (c)
        {
            case ';': token.eType = eSEMICOLON; ++m_szData; return token;
            case '=': token.eType = eEQUAL;     ++m_szData; return token;
            case ',': token.eType = eCOMMA;     ++m_szData; return token;
            case '{': token.eType = eLBRACE;    ++m_szData; return token;
            case '}': token.eType = eRBRACE;    ++m_szData; return token;
            case '(': token.eType = eLPAR;      ++m_szData; return token;
            case ')': token.eType = eRPAR;      ++m_szData; return token;
            case '>': token.eType = eGT;        ++m_szData; return token;
            case '<': token.eType = eLT;        ++m_szData; return token;
            case '+': token.eType = ePLUS;      ++m_szData; return token;
            case '-': token.eType = eMINUS;     ++m_szData; return token;
            case '*': token.eType = eMULT;      ++m_szData; return token;
            case '!': token.eType = eBANG;      ++m_szData; return token;
            case '&':
                if (m_szData[1] == '&') { token.eType = eAND; m_szData += 2; return token; }
                break;
            case '|':
                if (m_szData[1] == '|') { token.eType = eOR; m_szData += 2; return token; }
                break;
        }

        // Numbers
        bool isNumber = (c >= '0' && c <= '9') || c == '.';
        int i = 0;

        // 24 - max length of string
        while (i < MAX_TOKEN_LENGHT && *m_szData &&
            ((*m_szData >= '0' && *m_szData <= '9') ||
            (*m_szData >= 'a' && *m_szData <= 'z') ||
            (*m_szData >= 'A' && *m_szData <= 'Z') ||
            *m_szData == '.' || *m_szData == '_'))
        {
            token.acVal[i++] = *m_szData++;
        }

        if (i > 0)
        {
            // Out of buffer size
            if (i == MAX_TOKEN_LENGHT)
            {
                printf("Symbol too long in action mappings (max is %d) - %s", MAX_TOKEN_LENGHT - 1, m_szData - i);
                return token;
            }

            token.acVal[i] = '\0';

            if (isNumber)
            {
                token.eType = eNUMBER;
                token.fVal = static_cast<float>(atof(token.acVal));
                return token;
            }

            // Check for operations
                 if (strcmp(token.acVal, "tap") == 0)         token.eType = eTAP;
            else if (strcmp(token.acVal, "hold") == 0)        token.eType = eHOLD;
            else if (strcmp(token.acVal, "release") == 0)     token.eType = eRELEASE;
            else if (strcmp(token.acVal, "downedge") == 0)    token.eType = eDOWNEDGE;
            else if (strcmp(token.acVal, "fasttap") == 0)     token.eType = eFASTTAP;
            else if (strcmp(token.acVal, "doubleclick") == 0) token.eType = eDOUBLECLICK;
            else if (strcmp(token.acVal, "holddown") == 0)    token.eType = eHOLDDOWN;
            else if (strcmp(token.acVal, "clickhold") == 0)   token.eType = eCLICKHOLD;
            else if (strcmp(token.acVal, "press") == 0)       token.eType = ePRESS;
            else if (strcmp(token.acVal, "true") == 0)        token.eType = eVTRUE;
            else if (strcmp(token.acVal, "false") == 0)       token.eType = eVFALSE;
            else if (strcmp(token.acVal, "ana") == 0)         token.eType = eANA;
            else if (strcmp(token.acVal, "rel") == 0)         token.eType = eREL;
            else if (strcmp(token.acVal, "seq") == 0)         token.eType = eSEQ;
            else if (strcmp(token.acVal, "get") == 0)         token.eType = eGET;
            else                                              token.eType = eSYMBOL;

            return token;
        }

        // Skip single unknown char
        ++m_szData;
        return token;
    }
}