#pragma once

#include <Glacier/Serializer/IDictionary.h>
#include <Glacier/Serializer/ZToken.h>

#include <cstdint>


namespace Glacier
{
    struct ZFastDictionary : public IDictionary
    {
        // types
        struct cNode
        {
            struct cNode* m_Children[128];
            ZToken m_Token;

            uint32_t CountWordsLength(bool includeTerminators) const;
            ZToken GetLargestToken(ZToken largest) const;
            char* Unpack(char* pWrite, char* pWord, uint32_t lDepth, const char** pToken2Name) const;
        };

        // methods
        uint32_t CountWordsLength(bool includeTerminators) const;
        ZToken GetLargestToken() const;
        void Unpack(char* pWords, const char** pToken2Name) const;

        // members
        cNode  m_Root;
        ZToken m_NextToken;
    };
}
