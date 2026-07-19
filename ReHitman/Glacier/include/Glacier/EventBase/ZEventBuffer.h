#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZList.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/ZSTL/ZFixedSizeMemoryManager.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    class ZEventBuffer : public ZSerializable // total size is 0x1C
    {
    public:
        // static
        STATIC_CLASS_VAR(ZEventBuffer, ZEventBuffer*, m_Instance);

        // vtbl
        ~ZEventBuffer() override;
        void LoadObject(IInputSerializerStream &) override;
        void SaveObject(IOutputSerializerStream &) override;

        // methods
        static ZEventBuffer& Instance();
        ZEventBuffer(uint32_t iEventBufferSize);
        void InitEventBuffer(uint32_t lEventRamSize);
        void FreeEventBuffer();
        ZEventBase* AllocEventRam(uint32_t lEventSize);
        ZREF AllocRef(ZEventBase* pEvent);
        int AllocRefDirect(ZEventBase* pEvent, ZREF ref);
        ZEventBase* ConvEventRefToPtr(ZREF ref);
        void FreeEventRam(ZEventBase* pEvent);
        void FreeRef(ZREF ref);

        // members
        ZFixedSizeMemoryManager<ZEventBase*>* m_EventRefs;
        char* m_pEventRam;
		uint32_t m_lEventRamSize;
		ZOffsetAlloc* m_pEventAlloc;
        uint32_t m_lAllocatedEventsRam;
        uint32_t m_lNrAllocatedEvents;

    public:
	    // Custom API
	    static std::intptr_t GetGQC(Glacier::ZREF ref);

	    template <typename TEntity>
	    static TEntity* EventRefToInstance(Glacier::ZREF ref) {
		    return reinterpret_cast<TEntity*>(ZEventBuffer::GetGQC(ref));
	    }
    };
    RE_VERIFY_SIZE(ZEventBuffer, 0x1C); // verified
}
