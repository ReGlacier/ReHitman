#pragma once

#include <Glacier/Audio/ZSoundListener.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZSoundListener3D : public ZSoundListener
    {
    public:
        // vtbl
        virtual ZVector3* GetPosition();
        virtual ZVector3* GetFront();
        virtual ZVector3* GetTop();
        virtual ZVector3* GetEar();

        // members
        ZVector3 m_vFront;
        ZVector3 m_vTop;
        ZVector3 m_vEar;
        ZVector3 m_vPosition;
    };

    RE_VERIFY_OFFSET(ZSoundListener3D, m_vFront, 0x14);
    RE_VERIFY_OFFSET(ZSoundListener3D, m_vTop, 0x20);
    RE_VERIFY_OFFSET(ZSoundListener3D, m_vEar, 0x2C);
    RE_VERIFY_OFFSET(ZSoundListener3D, m_vPosition, 0x38);
    RE_VERIFY_SIZE(ZSoundListener3D, 0x44);
}
