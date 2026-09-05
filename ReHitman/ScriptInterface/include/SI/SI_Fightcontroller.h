#pragma once

#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    eAttackStyle Fightcontroller__Attackingtarget(ZREF rActor, int target, ZREF rTarget);
    void Fightcontroller__Requestcoverfire(ZREF rActor, ZREF rTarget);
    void Fightcontroller__Huntingtarget(ZREF rActor, int target, ZREF rTarget);
    void Fightcontroller__Standby(ZREF rActor, int target, ZREF rTarget);
    int Fightcontroller__Updatetargetinfo(ZREF rActor, ZREF rTarget, ZREF rTargetInfo);
    int Fightcontroller__Signoff(ZREF rActor);
    ZREF Fightcontroller__Querytarget(ZREF rActor, int target);
    ZREF Fightcontroller__Querytargetclose(ZREF rActor);
    bool Fightcontroller__Queryshouldattack(ZREF rActor, ZREF rTarget);
    bool Fightcontroller__Iscombathappening();
    void Fightcontroller__Requestnewposition(ZREF rPosition);
}
