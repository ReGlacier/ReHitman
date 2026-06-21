#pragma once

#include <cstdint>


namespace Glacier
{
    struct ZHashBase
    {
        // vtbl
        virtual void SetSize(unsigned int);
        virtual ~ZHashBase();
        virtual const void* GetArray();
        virtual unsigned int NodeSize();

        // members
        uint32_t* m_aTaken;
        uint32_t m_lSize;
        uint32_t m_lMinSize;
        uint32_t m_lCount;
        uint32_t m_lLongestSequence;
    };
    RE_VERIFY_SIZE(ZHashBase, 0x18);

    template <typename K, typename V>
    struct _SHashNode
    {
        K m_Key;
        V m_Value;
    };

    template <typename K, typename V> struct ZHash : ZHashBase
    {
        _SHashNode<K, V>* m_pArray;
        float m_fLoad;
    };

    struct ZPStrHash : ZHash<const char*, uint32_t>
    {
        virtual void IntValue(const char* const*, unsigned int*, unsigned int*);
        virtual bool Equals(const char* const*, const char* const*);
    };

    using ZIntHash = ZHash<int, unsigned int>;
    RE_VERIFY_SIZE(ZIntHash, 0x20);
}