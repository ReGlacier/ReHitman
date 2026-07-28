#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZSList.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct ZResolverList;

    struct ZMessageResolver : public ZSListNode<ZMessageResolver, 0>
    {
        // methods
        ZMessageResolver(const char* pszName);
        ~ZMessageResolver();

        operator uint32_t() const;

        static void ResolveAll();
        static void ClearAll();
        static ZResolverList& GetResolvers();

        // members
        const char* m_MessageName{nullptr};
        uint32_t m_MessageID{0u};
    };

    struct ZResolverList : public ZSList<ZMessageResolver, false, 0>
    {
        // methods
        ZResolverList();
        ~ZResolverList();

        void ResolveAll();
        void ClearAll();
        void Break();
        void Add(ZMessageResolver* resolver);

        // members
        bool m_bBroken { false };
    };
}