#pragma once


namespace Glacier
{
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

    static_assert(sizeof(ZOffsetAlloc) == 0x14, "Bad size of ZOffsetAlloc");
}