#pragma once

#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/ReGlacier.h>

#ifdef GetObject
#undef GetObject
#endif

namespace Glacier
{
    class ALLOCREF;

    class ZSoundObjectManager
    {
    public:
        // methods
        ZSoundObjectManager();
        ~ZSoundObjectManager();
        void Initialize();
        void Destroy();
        ZREF AllocateObject();
        void FreeObject(ZREF _reference);
        void Reset();
        void PopScene();
        void Invalidate();
        void CorrectDelayValue(float _correction);
        ZSoundObject* GetObject(uint32_t _index);
        ZSoundObject* ConvRefToPtr(ZREF _reference);
        int GetNumAllocated() const;

        // members
        ALLOCREF* m_pAllocRef;
        int32_t* m_pConvRefs;
        ZSoundObject* m_pObjects;
        int32_t m_FreeMemTab[512];
        int32_t m_lNumObjects;
        int32_t m_lNextFree;
    };

    RE_VERIFY_OFFSET(ZSoundObjectManager, m_FreeMemTab, 0x0C);
    RE_VERIFY_OFFSET(ZSoundObjectManager, m_lNumObjects, 0x80C);
    RE_VERIFY_SIZE(ZSoundObjectManager, 0x814);
}
