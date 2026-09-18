#pragma once

#include <Glacier/Serializer/ZTokenTable.h>


namespace Glacier
{
    struct ZDictionary;
    struct ZFastDictionary;
    struct ZInputStreamBase;
    struct ZOutputStreamBase;

    struct ZTokenTable_Serializerlib : public ZTokenTable
    {
        // vtbl
        // methods
        ~ZTokenTable_Serializerlib();
        ZTokenTable_Serializerlib();
        ZTokenTable_Serializerlib(ZFastDictionary& dict);
        ZTokenTable_Serializerlib(ZDictionary& dict);
        
        void Save(ZOutputStreamBase& stream);
        void Load(ZInputStreamBase& stream);

        // members
    };
}