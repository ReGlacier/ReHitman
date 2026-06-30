#pragma once

#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    struct ZRunMatPosAnim 
    {
        TIMETYPE m_fStTime;
        float m_fFPS;
        bool m_bLoop;
        CHUNKFILE* m_pChunkFile;
        float m_fFrameStar;
        float m_fFrameEnd;

        virtual void LoadObject(IInputSerializerStream&);
        virtual void SaveObject(IOutputSerializerStream&);
    };
    RE_VERIFY_SIZE(ZRunMatPosAnim, 0x1C);
}