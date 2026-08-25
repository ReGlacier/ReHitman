#include <Glacier/Animation/Manager.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier::Animation
{
    StateCache::StateCache(int lAnimCount, int lCacheLines)
    {
        if (lCacheLines)
        {
            m_CacheLines = (lCacheLines << 10) / sizeof(StateCacheEntry);
        }

        m_CacheEntry = (StateCacheEntry**)ZUniMemory::Allocate(sizeof(StateCacheEntry*) * lAnimCount);

        if (m_CacheLines)
        {
            m_Data = ZUniMemory::NewArray<StateCacheEntry>(m_CacheLines);
            m_AllocStack = (StateCacheEntry**)ZUniMemory::Allocate(sizeof(StateCacheEntry*) * m_CacheLines);
        }

        m_AllocPos = 0;
        m_AnimCount = lAnimCount;

        for (int i = 0; i < m_CacheLines; ++i)
        {
            m_AllocStack[i] = &m_Data[i];
            m_Data[i].m_Entry.m_Data = m_Data[i].m_Data;
        }

        for (int i = 0; i < lAnimCount; ++i)
        {
            m_CacheEntry[i] = nullptr;
        }

        m_Misses = 0;
        m_Hits = 0;
    }

    StateCache::~StateCache()
    {
        if (m_CacheEntry)
        {
            ZUniMemory::Delete(m_CacheEntry);
        }

        if (m_Data)
        {
            ZUniMemory::Delete(m_Data);
        }

        if (m_AllocStack)
        {
            ZUniMemory::Delete(m_AllocStack);
        }
    }

    StateCacheEntry* StateCache::FindEntry(int lCacheEntry, int16_t lBlock)
    {
        auto* pEntry = m_CacheEntry[lCacheEntry];
        if (pEntry)
        {
            while (pEntry->m_Entry.m_Block != lBlock)
            {
                pEntry = pEntry->m_Next;
                if (!pEntry)
                {
                    ++m_Misses;
                    return nullptr;
                }

                ++pEntry->m_Used;
                ++m_Hits;
                return pEntry;
            }
        }

        ++m_Misses;
        return nullptr;
    }

    StateCacheEntry* StateCache::AllocEntry(int lNextCacheEntry, int16_t lBlock)
    {
        if (m_AllocPos >= m_CacheLines)
            return nullptr;

        auto* pEntry = m_AllocStack[m_AllocPos++];
        pEntry->m_Entry.m_Block = lBlock;
        pEntry->m_Used = true;
        pEntry->m_Next = m_CacheEntry[lNextCacheEntry];
        m_CacheEntry[lNextCacheEntry] = pEntry;

        return pEntry;
    }

    void StateCache::Update()
    {
        for (int i = 0; i < m_AnimCount; ++i)
        {
            auto** pEntry = &m_CacheEntry[i];
            for (auto* j = *pEntry; j; j = j->m_Next)
            {
                if (j->m_Used)
                {
                    pEntry = &j->m_Next;
                    j->m_Used = false;
                }
                else
                {
                    *pEntry = j->m_Next;
                    m_AllocStack[m_AllocPos--] = j;
                }
            }
        }

        m_Misses = 0;
        m_Hits = 0;
    }
}
