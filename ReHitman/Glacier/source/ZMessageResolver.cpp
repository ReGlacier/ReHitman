#include <Glacier/ZMessageResolver.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZMessageResolver::ZMessageResolver(const char* pszName)
    {
        m_MessageName = pszName;
        m_MessageID = 0;

        auto& resolvers = ZMessageResolver::GetResolvers();
        resolvers.Add(this);
    }

    ZMessageResolver::~ZMessageResolver()
    {
        auto& resolvers = ZMessageResolver::GetResolvers();
        resolvers.Break();
    }

    ZMessageResolver::operator uint32_t() const
    {
        ZASSERT(m_MessageID);
        return m_MessageID;
    }

    void ZMessageResolver::ResolveAll()
    {
        GetResolvers().ResolveAll();
    }

    void ZMessageResolver::ClearAll()
    {
        GetResolvers().ClearAll();
    }

    ZResolverList& ZMessageResolver::GetResolvers()
    {
        static ZResolverList g_Resolvers;
        return g_Resolvers;
    }

    ZResolverList::ZResolverList()
        : ZSList<ZMessageResolver,false,0>()
        , m_bBroken(false)
    {
    }

    ZResolverList::~ZResolverList() = default;

    void ZResolverList::ResolveAll()
    {
        ZASSERT(m_bBroken == false);

        for (auto& resolver : *this)
        {
            resolver.m_MessageID = g_pEngineData->RegisterZMsg(resolver.m_MessageName, 0, __FILE__, __LINE__);
            ZASSERT(resolver.m_MessageID);
        }
    }

    void ZResolverList::ClearAll()
    {
        ZASSERT(m_bBroken == false);

        for (auto& resolver : *this)
        {
            resolver.m_MessageID = 0;
        }
    }

    void ZResolverList::Break()
    {
        m_bBroken = true;
    }

    void ZResolverList::Add(ZMessageResolver* resolver)
    {
        ZASSERT(resolver);
        AddLast(resolver);
    }
}