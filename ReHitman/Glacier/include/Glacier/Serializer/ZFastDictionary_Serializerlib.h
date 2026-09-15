#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/ZFastDictionary.h>
#include <Glacier/Serializer/IDictionary_Serializerlib.h>


namespace Glacier
{
    struct ZOutputStreamBase;
    
    struct ZFastDictionary_Serializerlib : public ZFastDictionary, public IDictionary_Serializerlib
    {
        // vtbl
        ~ZFastDictionary_Serializerlib() override;
        void SaveAsPackedDictionary(ZOutputStreamBase&) override;
        void SaveAsTokenTable(ZOutputStreamBase&) override;
        // methods
        ZFastDictionary_Serializerlib();
        // members (none)
    };
    RE_VERIFY_SIZE(ZFastDictionary_Serializerlib, 0x210); // Verified in PC
}