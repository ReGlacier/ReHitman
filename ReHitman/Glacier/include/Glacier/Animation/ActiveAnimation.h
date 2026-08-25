#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Animation/Fwd.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h> // ZPoolAllocator


namespace Glacier::Animation
{
    struct ActiveAnimation
    {
        // types
        using CallBack_t = bool(ZGEOM::*)(ActiveAnimation*, float, float, unsigned int);

        enum ECallType
        {
            CALL_ONCE = 0,
            CALL_ALWAYS = 1,
            CALL_LOOP = 2,
            CALL_LOOP_ALWAYS = 3,
        };

        // constants

        // static
        static char s_AnimationCallbackBuffer[34816];
        static ZPoolAllocator s_AnimationCallbackAllocator;

        // methods
        ActiveAnimation();
        ~ActiveAnimation();
        void Create(ZLNKOBJ* pLnkObj);
        void Destroy(bool bUpdateAlways);
        void LoadSave(ISerializerStream& stream, bool bSaving);
        void UpdateCallBacks();

        void AddCallBack(float, CallBack_t pCallback, unsigned int, long int, unsigned int);
        void AddCallBackAlways(float, CallBack_t pCallback, unsigned int);
        void AddCallBackLoop(float, CallBack_t pCallback, unsigned int);
        void AddMetaKeyCallBack(unsigned int, CallBack_t pCallback, unsigned int, long int, unsigned int);
        void AddMetaKeyCallBack(const char*, CallBack_t pCallback, unsigned int, long int);
        void SetCurrentFrame(float);

        // members
        Header* header{ nullptr };
        float frame{ 0.f };
        float startFrame{ 0.f };
        float endFrame{ 0.f };
        float blend{ 0.f };
        float deltaframe{ 0.f };
        int32_t mode{ 0 };
        int32_t sequenceId{ 0 };
        float m_fRemCallBackFrame{ 0.f };
        ZLNKOBJ* m_pLnkObj{ nullptr };
        ZPoolAllocLinkSortRefTab* m_prtCallBacks{ nullptr };
    };

    RE_VERIFY_SIZE(ActiveAnimation, 0x2C); // Verified by Animation::ActiveAnimation::Create & LoadSave
}
