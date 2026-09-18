#include <Glacier/GameBase/SSeerObject.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    bool SSeerObject::GetVisible() const
    {
        return lValue >> 7;
    }

    void SSeerObject::SetVisible(bool bVisible)
    {
        lValue &= ~0x80u;
        if (bVisible)
        {
            lValue |= 0x80u;
        }
    }

    int32_t SSeerObject::GetDeltaTime() const
    {
        return static_cast<int32_t>(lValue & 0x7F);
    }

    void SSeerObject::SetDeltaTime(int32_t lTime)
    {
        ZASSERT(lTime >= 0 && lTime <= 127);

        lValue &= 0x80;
        lValue |= static_cast<uint8_t>(lTime) & 0x7F;
    }
}
