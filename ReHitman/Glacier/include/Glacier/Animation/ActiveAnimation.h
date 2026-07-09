#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Animation/Fwd.h>


namespace Glacier::Animation
{
    struct ActiveAnimation 
    {
        Header* header;
        float frame;
        float startFrame;
        float endFrame;
        float blend;
        float deltaframe;
        int32_t mode;
        int32_t sequenceId;
        float m_fRemCallBackFrame;
        ZLNKOBJ* m_pLnkObj;
        ZPoolAllocLinkSortRefTab* m_prtCallBacks;

        void Create(ZLNKOBJ* pLnkObj);
    };

    RE_VERIFY_SIZE(ActiveAnimation, 0x2C); // Verified by Animation::ActiveAnimation::Create & LoadSave
}