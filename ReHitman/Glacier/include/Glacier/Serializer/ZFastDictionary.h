#pragma once

#include <Glacier/Serializer/IDictionary.h>
#include <Glacier/Serializer/ZToken.h>

#include <cstdint>


namespace Glacier
{
    struct ZPackedDictionary;
    
    struct ZFastDictionary : public IDictionary
    {
        // types
        struct cNode
        {
            // consts
            static constexpr size_t NUM_CHILD_NODES = 128;

            // members
            struct cNode* m_Children[NUM_CHILD_NODES];
            ZToken m_Token { 0 };

            // methods
            cNode();
            ~cNode();
            uint32_t CountWordsLength(bool includeTerminators) const;
            ZToken GetLargestToken(ZToken largest) const;
            char* Unpack(char* pWrite, char* pWord, uint32_t lDepth, const char** pToken2Name) const;
            int Pack(ZPackedDictionary& dict, int& count);
            uint32_t CountNodes() const;
        };


        // vtbl
        ~ZFastDictionary() override;
        ZToken GetToken(const char* word) override;

        // methods
        ZFastDictionary();
        uint32_t CountWordsLength(bool includeTerminators) const;
        ZToken GetLargestToken() const;
        void Unpack(char* pWords, const char** pToken2Name) const;
        ZFastDictionary::cNode* FindOrAdd(const char* pToken);
        uint32_t CountNodes() const;

        // members
        cNode  m_Root;
        ZToken m_NextToken;
    };
}
