#pragma once

#include <Glacier/ReGlacier.h>

#include <dsound.h>

namespace Glacier
{
    class ZEaxBase
    {
    public:
        // vtbl
        virtual ~ZEaxBase();
        virtual bool Init(IDirectSoundBuffer* _buffer, bool _setDefaults);
        virtual void Update();
        virtual bool QueryEAXSupport();
        virtual void GetDefaultValues() = 0;

        // methods
        ZEaxBase();

        // members
        IKsPropertySet* m_pPropertySet;
    };

    RE_VERIFY_OFFSET(ZEaxBase, m_pPropertySet, 0x04);
    RE_VERIFY_SIZE(ZEaxBase, 0x08);
}
