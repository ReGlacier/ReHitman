#pragma once

#include <Glacier/Serializer/ZCRCInputStream.h>


namespace Glacier
{
    /**
     * Engine-data variant of ZCRCInputStream used by PC's saved-game restore path
     * (`ZEngineDataBase::AllocSequence`, PC 0x45FCF0). PC constructs it through the
     * symbol named `ZMemoryInputStream` (0x4682E0), which first runs the normal
     * `ZCRCInputStream` constructor and then installs this class's vftable.
     *
     * No behaviour or fields differ from the base class in the PC build; the class
     * is a concrete stream-name/type marker for the engine-data save file.
     */
    struct ZEngineDataCRCInputStream : public ZCRCInputStream
    {
        // vtbl
        // The base destructor and Read are reused unchanged.

        // methods
        ZEngineDataCRCInputStream(IInputStream* stream)
            : ZCRCInputStream(stream)
        {
        }
    };
    RE_VERIFY_SIZE(ZEngineDataCRCInputStream, sizeof(ZCRCInputStream));
}
