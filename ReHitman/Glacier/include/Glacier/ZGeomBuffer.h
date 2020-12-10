#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZRTTI.h>

namespace Glacier
{
    // Class definition ZGeomBuffer
    class ZGeomBuffer
    {
    public:
        // vftable
        virtual void Release(bool); //#0000 at 00149258 org ZGeomBuffer::~ZGeomBuffer()
        virtual void PreSave(ISerializerStream &); //#0001 at 0014A104 org ZGeomBuffer::PreSave(ISerializerStream &)
        virtual void PostSave(ISerializerStream &); //#0002 at 0014A10C org ZGeomBuffer::PostSave(ISerializerStream &)
        virtual void PreLoad(ISerializerStream &); //#0003 at 0014A114 org ZGeomBuffer::PreLoad(ISerializerStream &)
        virtual void PostLoad(ISerializerStream &); //#0004 at 0014A11C org ZGeomBuffer::PostLoad(ISerializerStream &)
        virtual void PostProcess(unsigned int,unsigned int); //#0005 at 0014A124 org ZGeomBuffer::PostProcess(unsigned int,unsigned int)
        virtual void LoadSave(ISerializerStream &,bool); //#0006 at 0043C560 org ZSerializable::LoadSave(ISerializerStream &,bool)
        virtual void LoadObject(IInputSerializerStream &); //#0007 at 0014A12C org ZGeomBuffer::LoadObject(IInputSerializerStream &)
        virtual void SaveObject(IOutputSerializerStream &); //#0008 at 0014A3B4 org ZGeomBuffer::SaveObject(IOutputSerializerStream &)
        virtual void ExchangeObject(ISerializerStream &); //#0009 at 0043CC1C org ZSerializable::ExchangeObject(ISerializerStream &)
        virtual void SetToDefault(); //#0010 at 0043C568 org ZSerializable::SetToDefault(void)
        virtual Glacier::ZRTTI* GetTypeID(); //#0011 at 0043CC24 org ZSerializable::GetTypeID(void)

        // data
        ZEventBuffer* m_eventBuffer; //0x0004
        int32_t m_quartersCount; //0x0008
        ZGeomBufferPoolEntity* m_pool; //0x000C
        ZQElemsBuffer* m_quickElementsBuffer; //0x0010
        ZBaseGeomLists* m_baseGeomLists; //0x0014
    }; //End of ZGeomBuffer from 00550E38
}