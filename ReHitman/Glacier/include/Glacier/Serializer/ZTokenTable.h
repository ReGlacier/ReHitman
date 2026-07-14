#pragma once

#include <Glacier/Serializer/ZToken.h>


namespace Glacier
{
    struct ZDictionary;
    struct ZFastDictionary;
    struct ZInputStreamBase;
    struct ZOutputStreamBase;

    struct ZTokenTable
    {
        // vtbl - none
        // methods
        ~ZTokenTable();
        ZTokenTable();
        ZTokenTable(ZFastDictionary&);
        ZTokenTable(ZDictionary&);
        bool IsValidToken(ZToken token) const;
        const char* GetWord(ZToken token) const;

        // members
        ZToken m_LargestToken;
        char* m_Words;
        const char** m_Token2Name;
    };

    struct ZTokenTable_Serializerlib : public ZTokenTable
    {
        // vtbl
        // methods
        ~ZTokenTable_Serializerlib();
        ZTokenTable_Serializerlib();
        ZTokenTable_Serializerlib(ZDictionary&);
        ZTokenTable_Serializerlib(ZFastDictionary&);

        void Load(ZInputStreamBase*);
        void Save(ZOutputStreamBase*);
        // members
    };
}
