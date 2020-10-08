#pragma once

#include <Glacier/Glacier.h>
#include <Glacier/ZRTTI.h>

namespace Glacier
{
    class ZGeomBuffer
    {
    public:
        virtual void Release(bool);                         //#0
        virtual void PreSave(ISerializerStream &);          //#1
        virtual void PostSave(ISerializerStream &);         //#2
        virtual void PreLoad(ISerializerStream &);          //#3
        virtual void PostLoad(ISerializerStream &);         //#4
        virtual void PostProcess(uint,uint);                //#5
        virtual void LoadSave(ISerializerStream &,bool);    //#6
        virtual void LoadObject(IInputSerializerStream &);  //#7
        virtual void SaveObject(IOutputSerializerStream &); //#8
        virtual void ExchangeObject(ISerializerStream &);   //#9
        virtual void SetToDefault(void);                    //#10
        virtual ZRTTI* GetTypeID(void);                     //#11
    };
}