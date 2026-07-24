#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZTokenStream
    {
        static constexpr int MAX_TOKEN_LENGHT = 24;
        
        // types
        enum EToken 
        {
            eEND = 0,
            eINVALID = 1,
            eSEMICOLON = 2,
            eEQUAL = 3,
            eCOMMA = 4,
            eLBRACE = 5,
            eRBRACE = 6,
            eLPAR = 7,
            eRPAR = 8,
            eVTRUE = 9,
            eVFALSE = 10,
            eGET = 11,
            eOR = 12,
            eAND = 13,
            eGT = 14,
            eLT = 15,
            ePLUS = 16,
            eMINUS = 17,
            eMULT = 18,
            eUMINUS = 19,
            eBANG = 20,
            eSYMBOL = 21,
            eNUMBER = 22,
            eTAP = 23,
            eRELEASE = 24,
            eDOWNEDGE = 25,
            eFASTTAP = 26,
            eDOUBLECLICK = 27,
            eHOLDDOWN = 28,
            eCLICKHOLD = 29,
            ePRESS = 30,
            eHOLD = 31,
            eANA = 32,
            eREL = 33,
            eSEQ = 34,
        };

        struct ZTokenData
        {
            ZTokenStream::EToken eType;
            float fVal;
            char acVal[MAX_TOKEN_LENGHT];
        };

        // methods
        ZTokenStream(const char* psData);
        ~ZTokenStream();

        EToken Swallow();
        bool Swallow(ZTokenStream::EToken eToken, float& fValue);
        bool Swallow(ZTokenStream::EToken eToken, char* pStr);
        bool Swallow(ZTokenStream::EToken eToken);
        EToken Peek();
        ZTokenData ParseToken();

        // members
        const char* m_szData;
        ZTokenStream::ZTokenData kaTokens[1];
    };
    RE_VERIFY_SIZE(ZTokenStream, 0x24);
}