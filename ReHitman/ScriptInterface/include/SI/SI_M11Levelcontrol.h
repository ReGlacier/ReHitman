#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void M11Levelcontrol__Additemforcolicheck(ZREF rItem, ZREF rActor, float duration);
    void M11Levelcontrol__Removeitemforcolicheck(ZREF rItem);
    void M11Levelcontrol__Activatecoliforactor(ZREF rActor);
    void M11Levelcontrol__Deactivatecoliforactor(ZREF rActor);
    void M11Levelcontrol__Setarranger(ZREF rActor);
    void M11Levelcontrol__Setangel(ZREF rActor);
    void M11Levelcontrol__Usecomputer(ZREF rComputer, bool bEnabled);
    void M11Levelcontrol__Throwknife(ZREF rActor);
    void M11Levelcontrol__Operateprojector(ZREF rGeom);
    void M11Levelcontrol__Startprojector(bool bEnabled);
    void M11Levelcontrol__Removefromwaterbox(ZREF rGeom);
    float M11Levelcontrol__Karoketimeplayed();
    void M11Levelcontrol__Disablecrowdframeupdate(bool bDisabled);
}
