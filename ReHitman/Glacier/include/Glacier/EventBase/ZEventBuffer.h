#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZListNodeBase.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/ZFixedSizeMemoryManager.h>

namespace Glacier
{
    class ZEventBuffer // total size is 0x1C
    {
    public:
        // members
        ZFixedSizeMemoryManager<ZEventBase*>* m_EventRefs;
        char* m_pEventRam;
		uint32_t m_lEventRamSize;
		ZOffsetAlloc* m_pEventAlloc;
        uint32_t m_lAllocatedEventsRam;
        uint32_t m_lNrAllocatedEvents;

        /// === vftable ===
        virtual void Release(bool);
        virtual void PreSave(ISerializerStream&);
        virtual void PostSave(ISerializerStream&);
        virtual void PreLoad(ISerializerStream&);
        virtual bool PostLoad(ISerializerStream&);
        virtual bool PostProcess(unsigned int, unsigned int);
        virtual void LoadSave(ISerializerStream&, bool);
        virtual void LoadObject(IInputSerializerStream&);
        virtual void SaveObject(IOutputSerializerStream&);
        virtual void ExchangeObject(ISerializerStream&);
        virtual void SetToDefault();
        virtual unsigned int GetTypeID();

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