#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    class ZSoundBase
    {
    public:
        // vtbl
        virtual ~ZSoundBase();
        virtual bool Update();
        virtual void SetOwner(ZREF _owner);
        virtual ZREF GetOwner() const;

        // methods
        ZSoundBase();

        // members
        ZREF m_rOwner;
    };

    RE_VERIFY_OFFSET(ZSoundBase, m_rOwner, 0x04);
    RE_VERIFY_SIZE(ZSoundBase, 0x08);
}
