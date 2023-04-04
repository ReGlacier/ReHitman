#pragma once

#include <Glacier/ZListNodeBase.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZEventBase.h>

namespace Glacier
{
    class ZEventBuffer
    {
    public:
        // members
        char pad_0004[4]; //0x0004
        ZEventBase* m_events; //0x0008
        char pad_000C[4]; //0x000C
        ZOffsetAlloc* m_offsetAlloc; //0x0010
        char pad_0014[44]; //0x0014

        /// === vftable ===
        virtual void Release(bool);
	    virtual void PreSave(ZPackedInput*);
	    virtual void PostSave(ZPackedInput*);
	    virtual void PreLoad(ZPackedInput*);
	    virtual void PostLoad(ZPackedInput*);
	    virtual void PostProcess(int, int);
	    virtual void LoadSave(ZPackedInput*, bool);
	    virtual void LoadObject(IInputSerializerStream*);
	    virtual void SaveObject(IOutputSerializerStream*);
	    virtual void ExchangeObject(ZPackedInput*);
	    virtual void SetToDefault();
	    virtual int GetTypeID();

    public:
	    // Custom API
	    static std::intptr_t GetGQC(Glacier::ZREF ref);

	    template <typename TEntity>
	    static TEntity* EventRefToInstance(Glacier::ZREF ref) {
		    return reinterpret_cast<TEntity*>(ZEventBuffer::GetGQC(ref));
	    }
    };

}