#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier::Animation
{
    struct ZNameList
    {
        // members
        char* m_Names { nullptr };
        int m_Size { 0 };
        int m_Count { 0 };

        // methods
        ZNameList() = default;
        ~ZNameList() = default;

        int GetId(const char* pszAnimName, int iNoneIndex);
        const char* GetName(int id, int none);
    };
    RE_VERIFY_SIZE(ZNameList, 0xC);
}
