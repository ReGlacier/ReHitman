#pragma once

#include <BloodMoney/Game/ZHM3Actor.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Common.h>
#include <Glacier/ZREF.h>

namespace Glacier
{
    using namespace Hitman::BloodMoney;

    void Zactor__Setscriptpaused(ZREF rActor, bool paused);
    void Zactor__Setmoveset(ZREF rActor, int moveSet, int moveSetType);
    void Zactor__Stopmovementatdistance(ZREF rActor, float distance);
    void Zactor__Setmovespeedmultiplier(ZREF rActor, float speedMultiplier);
    float Zactor__Getactorspeed(ZREF rActor, ZREF rTarget);
    void Zactor__Dontstop(ZREF rActor, bool dontStop);
    int Zactor__Movetoposq(ZREF rActor, v3 position, float distance, int moveSet, int moveSetType);
    int Zactor__Movetolocationq(ZREF rActor, ZREF rLocation, float distance, int moveSet, int moveSetType);
    void Zactor__Stoppath(ZREF rActor);
    void Zactor__Setstopdistance(ZREF rActor, float stopDistance);
    void Zactor__Setpathnotify(ZREF rActor, float notifyDistance);
    void Zactor__Setupdateposition(ZREF rActor, bool updatePosition);
    void Zactor__Lockmovement(ZREF rActor);
    void Zactor__Unlockmovement(ZREF rActor);
    void Zactor__Clearreservation(ZREF rActor);
    void Zactor__Ignorereservations(ZREF rActor);
    void Zactor__Usereservations(ZREF rActor);
    bool Zactor__Isragdollactive(ZREF rActor, ZREF rTarget);
    bool Zactor__Isdead(ZREF rActor);
    bool Zactor__Isunconscious(ZREF rActor);
    bool Zactor__Isawake(ZREF rActor);
    int Zactor__Getactorstate(ZREF rActor);
    void Zactor__Setawake(ZREF rActor);
    void Zactor__Setunconscious(ZREF rActor);
    void Zactor__Setsleeping(ZREF rActor);
    void Zactor__Setdead(ZREF rActor);
    void Zactor__Lookat(ZREF rActor, ZREF rTarget);
    ZREF Zactor__Getlookat(ZREF rActor);
    void Zactor__Enablelookat(ZREF rActor, bool enabled);
    bool Zactor__Islookatenabled(ZREF rActor);
    void Zactor__Setactorposdir(ZREF rActor, v3 position, v3 direction);
    void Zactor__Copytoref(ZREF rActor, ZREF rReference);
    void Zactor__Setvisfov(ZREF rActor, float fov);
    bool Zactor__Isonstairs(ZREF rActor);
    void Zactor__Setshootintoground(ZREF rActor, bool enabled);
    void Zactor__Setshootintogroundinfullbody(ZREF rActor, bool enabled);
    float Zactor__Getboidspeed(ZREF rActor);
    void Zactor__Inactivateboid(ZREF rActor);
    void Zactor__Placeactorinmovinggroup(ZREF rActor, ZREF rGroup, ZREF rReference);
    void Zactor__Removeactorfrommovinggroup(ZREF rActor, ZREF rGroup);
    int Zactor__Getnumoftransitions(ZREF rActor, eMoveSet moveSet);
    void Zactor__Setnumoftransitions(ZREF rActor, eMoveSet moveSet, int numTransitions);
    void Zactor__Setfootwearmaterial(ZREF rActor, int material);
    void Zactor__Setmovepoolweight(ZREF rActor, float weight);
    void Zactor__Sethandtarget(ZREF rActor, int hand, ZREF rTarget, float weight);
    int Zactor__Getvariant(ZREF rActor);
    void Zactor__Setvariant(ZREF rActor, int variant);
    void Zactor__Addtoforcedrawoffscreenlist(ZREF rActor);
    void Zactor__Removefromforcedrawoffscreenlist(ZREF rActor);
    void Zactor__Turnactor(ZREF rActor, float angle);
    void Zactor__Setcanopendoors(ZREF rActor, bool canOpenDoors);
}
