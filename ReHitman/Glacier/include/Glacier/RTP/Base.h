#pragma once

namespace Glacier::RTP
{
    struct cNode
    {
        struct cNode* m_Next;
        const char* m_Name;
        unsigned int m_Filter;
    };

    struct ZPropertyInfo
    {
        cNode* First;
        ZPropertyInfo* Super;
        const char* Name;
    };
}