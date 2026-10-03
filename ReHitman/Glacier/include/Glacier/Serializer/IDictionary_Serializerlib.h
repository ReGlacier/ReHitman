#pragma once


namespace Glacier
{
    struct ZOutputStreamBase;

    struct IDictionary_Serializerlib
    {
        virtual ~IDictionary_Serializerlib();
        virtual void SaveAsPackedDictionary(ZOutputStreamBase&) = 0;
        virtual void SaveAsTokenTable(ZOutputStreamBase&) = 0;
    };
}
