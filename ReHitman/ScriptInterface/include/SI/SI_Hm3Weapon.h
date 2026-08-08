#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    ZREF Hm3Weapon__Getweapontemplate(ZREF rWeapon);
    int Hm3Weapon__Getweapontype(ZREF rWeapon);
    double Hm3Weapon__Gettimebetweenshots(ZREF rWeapon);
    int Hm3Weapon__Getweaponoperations(ZREF rWeapon);
    bool Hm3Weapon__Weaponfiresprojectiles(ZREF rWeapon);
    int Hm3Weapon__Getprojectilesinmagazine(ZREF rWeapon);
    bool Hm3Weapon__Emptyweapon(ZREF rWeapon);
    bool Hm3Weapon__Setprojectilesinmagazine(ZREF rWeapon, char projectiles, bool enabled);
    bool Hm3Weapon__Weaponissmall(ZREF rWeapon);
    bool Hm3Weapon__Issniper(ZREF rWeapon);
    bool Hm3Weapon__Isdetectable(ZREF rWeapon);
}
