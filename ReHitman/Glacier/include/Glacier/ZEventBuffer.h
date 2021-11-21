#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier {
    class ZEventBuffer {
    public:
        //vftable
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

        // Methods

    public:
        // Custom API
        static std::intptr_t GetGQC(Glacier::ZREF ref);

        template <typename TEntity>
        static TEntity* EventRefToInstance(Glacier::ZREF ref) {
            return reinterpret_cast<TEntity*>(ZEventBuffer::GetGQC(ref));
        }

        // Data (size is 0x1C)
        int m_field4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
        int m_field18;
    };
}