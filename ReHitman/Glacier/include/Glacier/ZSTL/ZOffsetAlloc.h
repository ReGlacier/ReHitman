#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    class REFTAB;

    class ZOffsetAlloc
    {
    public:
        struct ZLink
        {
            unsigned int m_lOffset;
            unsigned int m_lSize;
        };

        ZLink* m_pLinks;
        int m_lNrLinks;
        int m_lMaxNrLinks;
        bool m_bUserSuppliedLinkBuffer;
        bool m_pad[3];

    public:
        virtual ~ZOffsetAlloc();
        virtual int GetFreeTotal() const;
        virtual void GetFreeList(REFTAB* pRefTab) const;
    };
    RE_VERIFY_SIZE(ZOffsetAlloc, 0x14);
}