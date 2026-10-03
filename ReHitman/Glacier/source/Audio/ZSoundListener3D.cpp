#include <Glacier/Audio/ZSoundListener3D.h>

namespace Glacier
{
    ZVector3* ZSoundListener3D::GetPosition()
    {
        return &m_vPosition;
    }

    ZVector3* ZSoundListener3D::GetFront()
    {
        return &m_vFront;
    }

    ZVector3* ZSoundListener3D::GetTop()
    {
        return &m_vTop;
    }

    ZVector3* ZSoundListener3D::GetEar()
    {
        return &m_vEar;
    }
}
