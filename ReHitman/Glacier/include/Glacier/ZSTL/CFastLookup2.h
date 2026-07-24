#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct ZValTree;
    class LINKREFTAB;

    struct SFastLookupLink
    {
        const char* m_pLookup{nullptr};
        int32_t m_lLookupLen{0};
        uint32_t m_lValue{0};
    };
    RE_VERIFY_SIZE(SFastLookupLink, 0xC);

    struct CFastLookup2
    {
        // methods
        CFastLookup2(int lRefTabSize);
        ~CFastLookup2();

        int CalcChkSum(const char* pLookup, int lLookupLen);
        void Clear();
        SFastLookupLink* FindFastLink(LINKREFTAB* pList, const char* pLookup, int lLookupLen);
        uint32_t Get(const char* pLookup);
        int GetLowerCase(const char* pLookup);
        SFastLookupLink* PrepareFastLink(const char* pLookup, int lLookupLen);
        uint32_t Remove(const char* pLookup, int lLookupLen);
        void RemoveLowerCase(const char* pLookup);
        void Reset();
        void Set(const char* pLookup, uint32_t lVal);
        void SetLowerCase(const char* pLookup, uint32_t lVal);

        // members
        ZValTree* m_pChkSumTree{nullptr};
        bool m_bStatic{false};
        bool m_bRuntime{false};
    };
    RE_VERIFY_SIZE(CFastLookup2, 0x8);
}
