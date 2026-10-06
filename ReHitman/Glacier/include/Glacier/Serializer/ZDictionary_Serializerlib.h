#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZDictionary.h>


namespace Glacier
{
    struct ZDictionary_Serializerlib : public ZDictionary, public IDictionary_Serializerlib
    {
        // vtbl - need sync with PC, looks correcty
        ~ZDictionary_Serializerlib() override;
        void SaveAsPackedDictionary(ZOutputStreamBase&) override;
        void SaveAsTokenTable(ZOutputStreamBase&) override;

        // methods
        ZDictionary_Serializerlib();

        // members - none
    };
    RE_VERIFY_SIZE(ZDictionary_Serializerlib, 0x2C); // Verified in PC
}
