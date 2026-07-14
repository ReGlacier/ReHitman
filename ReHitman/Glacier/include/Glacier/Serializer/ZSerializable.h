#pragma once

#include <cstdint>


namespace Glacier
{
    struct ISerializerStream;
    struct IInputSerializerStream;
    struct IOutputSerializerStream;

    struct ZSerializableBase
    {};
    
    struct ZSerializable : public ZSerializableBase
    {
        virtual ~ZSerializable() {}
        virtual void PreSave(ISerializerStream&) {}
        virtual void PostSave(ISerializerStream&) {}
        virtual void PreLoad(ISerializerStream&) {}
        virtual bool PostLoad(ISerializerStream&) { return true; }
        virtual bool PostProcess(uint32_t, uint32_t) { return true; }
        virtual void LoadSave(ISerializerStream&, bool) {} // Removed in MiniNinjas, but not in HBM lol
        virtual void LoadObject(IInputSerializerStream &) {}
        virtual void SaveObject(IOutputSerializerStream &) {}
        virtual void ExchangeObject(ISerializerStream &) {}
        virtual void SetToDefault() {}
        virtual uint32_t GetTypeID() { return 0; }
    };
}