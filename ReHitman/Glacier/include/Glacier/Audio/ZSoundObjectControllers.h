#pragma once

#include <Glacier/Audio/ZAudioType.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/STLport.h>


namespace Glacier::SoundObject
{
    struct ZUpdater
    {
        virtual ~ZUpdater() = default;
    };

    struct ZControllerOwner
    {
        ZAudioTypeBase::ZPackedBase* m_pPackedAudioObject;
        stlp::vector<ZUpdater*> m_Controllers;
    };

    class ZControllers
    {
    public:
        ZControllers();
        ~ZControllers();
        void Clear();
        bool Exists(ZAudioTypeBase::ZPackedBase& _packed) const;

        stlp::vector<ZControllerOwner*> m_Controllers;
        stlp::vector<ZControllerOwner*> m_RemoveThese;
    };

    RE_VERIFY_SIZE(ZControllerOwner, 0x10);
    RE_VERIFY_SIZE(ZControllers, 0x18);
}
