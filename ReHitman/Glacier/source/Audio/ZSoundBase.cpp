#include <Glacier/Audio/ZSoundBase.h>

namespace Glacier
{
    ZSoundBase::ZSoundBase() = default;

    ZSoundBase::~ZSoundBase() = default;

    bool ZSoundBase::Update()
    {
        return true;
    }

    void ZSoundBase::SetOwner(ZREF _owner)
    {
        m_rOwner = _owner;
    }

    ZREF ZSoundBase::GetOwner() const
    {
        return m_rOwner;
    }
}
