#pragma once

#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    void Guardquartercontroller__Initializeweaponstorage(ZREF rGuardQuarterControl, ZREF rWeapon);
    void Guardquartercontroller__Registeractor(ZREF rGuardQuarterControl, ZREF rActor);
    void Guardquartercontroller__Delegatetask(ZREF rGuardQuarterControl, sGQCTask task);
    void Guardquartercontroller__Taskdone(ZREF rGuardQuarterControl, ZREF rActor);
    void Guardquartercontroller__Accepttask(ZREF rGuardQuarterControl, sGQCTask task);
    void Guardquartercontroller__Actornotavailable(ZREF rGuardQuarterControl, ZREF rActor);
    ZREF Guardquartercontroller__Getclosestbodybagbox(ZREF rGuardQuarterControl, ZREF rGeom);
    void Guardquartercontroller__Decreasebodybagboxcapacity(ZREF rGuardQuarterControl, ZREF rBodyBagBox);
    bool Guardquartercontroller__Canaddweapontostorage(ZREF rGuardQuarterControl, ZREF rWeapon);
    void Guardquartercontroller__Placeweaponinstorage(ZREF rGuardQuarterControl, ZREF rWeapon);
    void Guardquartercontroller__Removeweaponfromstorage(ZREF rGuardQuarterControl, ZREF rWeapon);
    void Guardquartercontroller__Getstorageposdirref(ZREF rGuardQuarterControl, v3& position, v3& direction, ZREF& rStorageRef);
    bool Guardquartercontroller__Isweaponinstorage(ZREF rGuardQuarterControl, ZREF rWeapon);
    ZREF Guardquartercontroller__Getweaponreffromstorage(ZREF rGuardQuarterControl, ZREF rWeapon);
}
