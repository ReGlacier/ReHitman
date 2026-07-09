#pragma once

#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <cstdint>


namespace Glacier
{
    struct NotCopyable
    {
        NotCopyable(const NotCopyable&) = delete;
        NotCopyable& operator=(const NotCopyable&) = delete;
    };

    template <typename T>
    struct ZFactory : public NotCopyable
    {
        struct ProducerData : public ZGEOMCLASSINFO
        {
            uint32_t m_ProducerId;
            T*(__cdecl* m_pCreator)(ZGEOMCLASSINFO*);
            ProducerData* m_pNext;
        };

        struct Iterator
        {
            ProducerData* m_p;
            uint32_t m_HashIndex;
            ZFactory<T>* m_Factory;
        };

        ProducerData* m_DataMap[16];
    };

    // Same template instance to check final fit size
    RE_VERIFY_SIZE(ZFactory<int>, 0x40);

    template <typename T>
    struct ZFactoryProducer
    {
        ZFactory<T>::ProducerData m_Data;
    };
}