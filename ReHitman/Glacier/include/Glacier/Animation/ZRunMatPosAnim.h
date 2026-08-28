#pragma once

#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class ZRunMatPosAnim
    {
    public:
        // vtbl
        virtual void LoadObject(IInputSerializerStream&);
        virtual void SaveObject(IOutputSerializerStream&);

        // methods
        ZRunMatPosAnim();
        void SetStartTime(const TIMETYPE& sTimeType);
        void SetFrameStart(int lFrameStart);
        void SetFrameEnd(int lFrameEnd);
        float Run(TIMETYPE t, float*, float*, float);
        float FPS() const;
        float GetStart() const;
        float GetEnd() const;
        float GetAnimEndTime() const;
        void SetChunkFile(CHUNKFILE* pChunkFile);
        void Create(CHUNKFILE* pChunkFile, TIMETYPE startTime, bool, float);

        // members
        TIMETYPE m_fStTime;
        float m_fFPS;
        bool m_bLoop;
        CHUNKFILE* m_pChunkFile;
        float m_fFrameStart;
        float m_fFrameEnd;
    };
    RE_VERIFY_SIZE(ZRunMatPosAnim, 0x1C);
}
