#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <G1ConfigurationService.h>
#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <malloc.h>


namespace Glacier 
{
    ZEventBuffer& ZEventBuffer::Instance()
    {
        ZASSERT(ZEventBuffer::m_Instance != nullptr);

        return *ZEventBuffer::m_Instance;
    }

    ZEventBuffer::~ZEventBuffer()
    {
        assert(m_Instance==this);

        FreeEventBuffer();

        if (m_EventRefs)
        {
            ZUniMemory::Delete(m_EventRefs);            
            m_EventRefs = nullptr;
        }

        ZEventBuffer::m_Instance = nullptr;
    }

    void ZEventBuffer::LoadObject(IInputSerializerStream &stream)
    {
        uint32_t count = m_EventRefs->Count();
        stream.ExchangeContainer("NumberOfEvents", count);

        // Original PC/PS2 builds use stack arrays here via alloca.
        uint32_t* refs = static_cast<uint32_t*>(_alloca(sizeof(uint32_t) * count));
        const char** refTypes = static_cast<const char**>(_alloca(sizeof(const char*) * count));

        stream.ExchangeArray("REFs", refs, count);
        stream.ExchangeArray("REFTypes", refTypes, count);

        ZEventBase** eventRefsBuffer = m_EventRefs->GetBufferPtr();
        uint32_t currentIndex = 0;

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t requestedRef = refs[i];
            ZASSERT(requestedRef != 0xFFFFFFFF);

            uint32_t requestedIndex = m_EventRefs->REF2Index(requestedRef);

            while (currentIndex < requestedIndex)
            {
                ZEventBase** currentSlot = &eventRefsBuffer[currentIndex];

                if (m_EventRefs->IsAllocated(currentSlot))
                {
                    (*currentSlot)->Delete();
                }

                ++currentIndex;
            }

            ZEventBase** eventSlot = &eventRefsBuffer[currentIndex];
            uint32_t currentRef = m_EventRefs->Ptr2REF(eventSlot);

            if (currentRef != requestedRef)
            {
                if (currentRef != 0xFFFFFFFF)
                {
                    (*eventSlot)->Delete();
                }

                const char* refType = refTypes[i];
                if (refType[0] != '@' || refType[1])
                {
                    ZEventBase::m_DirectRef = requestedRef + 1;

                    if (refType[0])
                    {
                        std::ignore = ZBaseConRout::GetFactory().Create(refType);
                    }
                    else
                    {
                        // The constructor consumes m_DirectRef and registers this event in the requested ref slot.
                        std::ignore = new ZEventBase();
                    }
                }
            }

            ++currentIndex;
        }
    }

    void ZEventBuffer::SaveObject(IOutputSerializerStream &stream)
    {
        uint32_t count = m_EventRefs->Count();

        // I hate alloca
        uint32_t* refs = static_cast<uint32_t*>(alloca(sizeof(uint32_t) * count));
        const char** refTypes = static_cast<const char**>(alloca(sizeof(const char*) * count));

        uint32_t i = 0;

        for (auto it = m_EventRefs->Begin(); it != m_EventRefs->End(); ++it)
        {
            ZEventBase** ppEvent = reinterpret_cast<ZEventBase**>(it.Get());
            ZEventBase* event = *ppEvent;

            refs[i] = m_EventRefs->Ptr2REF(ppEvent);

            if (event->SkipSave())
            {
                refTypes[i] = "@";
            }
            else
            {
                const char* eventName = event->EventName();
                refTypes[i] = eventName ? eventName : "";
            }

            ++i;
        }

        stream.SaveContainer("NumberOfEvents", count);
        stream.ExchangeArray("REFs", refs, count);
        stream.ExchangeArray("REFTypes", refTypes, count);
    }
    
    ZEventBuffer::ZEventBuffer(uint32_t lEventBufferSize)
        : m_pEventRam(nullptr)
        , m_lEventRamSize(0)
        , m_pEventAlloc(nullptr)
        , m_lAllocatedEventsRam(0)
        , m_lNrAllocatedEvents(0)
    {
        InitEventBuffer((lEventBufferSize + 0xF) & 0xFFFFFFF0);

        m_EventRefs = ZUniMemory::New<ZFixedSizeMemoryManager<ZEventBase*>>(0x2000);
        m_Instance = this;
    }

    void ZEventBuffer::InitEventBuffer(uint32_t lEventRamSize)
    {
        if (m_pEventRam || m_pEventAlloc)
        {
            FreeEventBuffer();
        }

        m_lEventRamSize = lEventRamSize;
        m_pEventRam = (char*)ZUniMemory::Allocate(sizeof(char) * lEventRamSize);
        //uint32_t lStartOffset, uint32_t lEndOffset, uint32_t lMaxNrAllocs, ZOffsetAlloc::ZLink *pBuffer, EAllocType eType
        m_pEventAlloc = ZUniMemory::New<ZOffsetAlloc>(0, m_lEventRamSize, 0x200, nullptr, EAllocType::DEFAULT_MEM);
    }

    void ZEventBuffer::FreeEventBuffer()
    {
        if (m_pEventRam)
        {
            ZUniMemory::Free(m_pEventRam);
            m_pEventRam = nullptr;
        }

        if (m_pEventAlloc)
        {
            ZUniMemory::Delete(m_pEventAlloc);
            m_pEventAlloc = nullptr;
        }

        m_lEventRamSize = 0;
        m_lAllocatedEventsRam = 0;
    }

    ZEventBase* ZEventBuffer::AllocEventRam(uint32_t lEventSize)
    {
        uint32_t lAlignedSize = (lEventSize + 3) & 0xFFFFFFFC;
        ZASSERT(lAlignedSize < 0x40000);

        m_lAllocatedEventsRam += lAlignedSize;
        
        uint32_t lAllocSize = (lAlignedSize + 0xF) & 0xFFFFFFF0;
        uint32_t lOffset = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m_pEventAlloc->Alloc(lAllocSize, true)));
        ZEventBase* pEvent = reinterpret_cast<ZEventBase*>(m_pEventRam + lOffset);
        // Don't call constructor here! It will be called from outside
        pEvent->m_lEventAllocSize = lAlignedSize >> 2;
        ++m_lNrAllocatedEvents;
        return pEvent;
    }

    ZREF ZEventBuffer::AllocRef(ZEventBase* pEvent)
    {
        ZEventBase** event = m_EventRefs->Alloc();
        ZASSERT(event);
        *event = pEvent;

        auto ref = m_EventRefs->Ptr2REF((void*)event);
        ZASSERT(ref < 0x1000000);

        return ref;
    }

    int ZEventBuffer::AllocRefDirect(ZEventBase* pEvent, ZREF ref)
    {
        *m_EventRefs->AllocDirect(ref) = pEvent;
        return ref + 1;
    }
    
    ZEventBase* ZEventBuffer::ConvEventRefToPtr(ZREF ref)
    {
        auto** pEnt = m_EventRefs->REF2Ptr(ref);
        if (!pEnt)
            return nullptr;

        return *pEnt;
    }

    void ZEventBuffer::FreeEventRam(ZEventBase* pEvent)
    {
        const uint32_t lAlignedSize = pEvent->m_lEventAllocSize << 2; 
        size_t offset = (char *)pEvent - this->m_pEventRam;
        
        m_pEventAlloc->Free(offset, (lAlignedSize + 15) & 0xFFFFFFF0);
        m_lAllocatedEventsRam -= lAlignedSize;
        --m_lNrAllocatedEvents;
    }

    void ZEventBuffer::FreeRef(ZREF ref)
    {
        auto** event = m_EventRefs->REF2Ptr(ref);
        ZASSERT(event);
        m_EventRefs->Free(event);
    }

    STATIC_CLASS_VAR_IMPL(ZEventBuffer, ZEventBuffer*, m_Instance, 0x008BA0B8, nullptr);
}
