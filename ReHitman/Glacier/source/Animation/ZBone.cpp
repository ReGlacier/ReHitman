#include <Glacier/Animation/ZBone.h>


namespace Glacier
{
    ZBone::ZBone()
        : _Mat{
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f
        }
        , _Pos{
            0.0f, 0.0f, 0.0f
        }
    {}

    ZBone::ZBone(const ZBone& copy)
    {
        // A little bit risky, but fast as fuck
        memcpy(this, &copy, sizeof(ZBone));
    }

    ZBone& ZBone::operator=(const ZBone& copy)
    {
        // A little bit risky, but fast as fuck
        memcpy(this, &copy, sizeof(ZBone));
        return *this;
    }
}