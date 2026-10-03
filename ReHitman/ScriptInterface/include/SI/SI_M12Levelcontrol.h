#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void M12Levelcontrol__Sionhighnoon();
    void M12Levelcontrol__Siafterhighnoon();
    bool M12Levelcontrol__Sihighnoonactive();
    bool M12Levelcontrol__Sihighnoonsilenced();
    bool M12Levelcontrol__Isweaponremotebomb(ZREF rWeapon);
    void M12Levelcontrol__Displaybombhint();
    void M12Levelcontrol__Moveobject(ZREF rObject, ZREF rPosition, ZREF rDirection, float fTime, float fBlendTime);
    void M12Levelcontrol__Disablehitmancontrols(bool bDisabled);
}
