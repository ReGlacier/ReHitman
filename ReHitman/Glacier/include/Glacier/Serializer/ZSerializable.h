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
        virtual void PreSave(ISerializerStream& stream) {}
        virtual void PostSave(ISerializerStream& stream) {}
        virtual void PreLoad(ISerializerStream& stream) {}
        virtual bool PostLoad(ISerializerStream& stream) { return true; }
        virtual bool PostProcess(uint32_t filter, uint32_t pass) { return true; }
        virtual void LoadSave(ISerializerStream& stream, bool bSaving) {} // Removed in MiniNinjas, but not in HBM lol
        virtual void LoadObject(IInputSerializerStream& stream) {}
        virtual void SaveObject(IOutputSerializerStream& stream) {}
        virtual void ExchangeObject(ISerializerStream& stream) {}
        virtual void SetToDefault() {}
        virtual uint32_t GetTypeID() { return 0; }
    };
}