#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct _AsyncCall_Struct
    {
        _AsyncCall_Struct* pNext;
        float m_fStoredNextRun;
        _LocalVarEntry* m_pLVE;
    };
    RE_VERIFY_SIZE(_AsyncCall_Struct, 0xC);

    using AsyncCall_Struct = _AsyncCall_Struct;
}