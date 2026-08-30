#include <Glacier/Audio/ZSoundObjectControllers.h>

#include <Glacier/ZUniMemory.h>

namespace Glacier::SoundObject
{
    ZControllers::ZControllers() = default;

    ZControllers::~ZControllers()
    {
        Clear();
    }

    void ZControllers::Clear()
    {
        for (ZControllerOwner* owner : m_Controllers)
            m_RemoveThese.push_back(owner);
        m_Controllers.clear();

        for (ZControllerOwner* owner : m_RemoveThese)
        {
            if (!owner)
                continue;
            for (ZUpdater* updater : owner->m_Controllers)
                ZUniMemory::Delete(updater);
            ZUniMemory::Delete(owner);
        }
        m_RemoveThese.clear();
    }

    bool ZControllers::Exists(ZAudioTypeBase::ZPackedBase& _packed) const
    {
        for (const ZControllerOwner* owner : m_Controllers)
        {
            if (owner && owner->m_pPackedAudioObject == &_packed)
                return true;
        }
        return false;
    }
}
