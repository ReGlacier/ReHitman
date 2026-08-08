#include <SI/SI_ZActor.h>

namespace Glacier
{
    void Zactor__Setscriptpaused(ZREF rActor, bool paused)
    {
        // TODO: Finish me
    }

    void Zactor__Setmoveset(ZREF rActor, int moveSet, int moveSetType)
    {
        // TODO: Finish me
    }

    void Zactor__Stopmovementatdistance(ZREF rActor, float distance)
    {
        // TODO: Finish me
    }

    void Zactor__Setmovespeedmultiplier(ZREF rActor, float speedMultiplier)
    {
        // TODO: Finish me
    }

    float Zactor__Getactorspeed(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zactor__Dontstop(ZREF rActor, bool dontStop)
    {
        // TODO: Finish me
    }

    int Zactor__Movetoposq(ZREF rActor, v3 position, float distance, int moveSet, int moveSetType)
    {
        // TODO: Finish me
        return 0;
    }

    int Zactor__Movetolocationq(ZREF rActor, ZREF rLocation, float distance, int moveSet, int moveSetType)
    {
        // TODO: Finish me
        return 0;
    }

    void Zactor__Stoppath(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Setstopdistance(ZREF rActor, float stopDistance)
    {
        // TODO: Finish me
    }

    void Zactor__Setpathnotify(ZREF rActor, float notifyDistance)
    {
        // TODO: Finish me
    }

    void Zactor__Setupdateposition(ZREF rActor, bool updatePosition)
    {
        // TODO: Finish me
    }

    void Zactor__Lockmovement(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Unlockmovement(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Clearreservation(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Ignorereservations(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Usereservations(ZREF rActor)
    {
        // TODO: Finish me
    }

    bool Zactor__Isragdollactive(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
        return false;
    }

    bool Zactor__Isdead(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zactor__Isunconscious(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    bool Zactor__Isawake(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    int Zactor__Getactorstate(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zactor__Setawake(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Setunconscious(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Setsleeping(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Setdead(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Lookat(ZREF rActor, ZREF rTarget)
    {
        // TODO: Finish me
    }

    ZREF Zactor__Getlookat(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zactor__Enablelookat(ZREF rActor, bool enabled)
    {
        // TODO: Finish me
    }

    bool Zactor__Islookatenabled(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    void Zactor__Setactorposdir(ZREF rActor, v3 position, v3 direction)
    {
        // TODO: Finish me
    }

    void Zactor__Copytoref(ZREF rActor, ZREF rReference)
    {
        // TODO: Finish me
    }

    void Zactor__Setvisfov(ZREF rActor, float fov)
    {
        // TODO: Finish me
    }

    bool Zactor__Isonstairs(ZREF rActor)
    {
        // TODO: Finish me
        return false;
    }

    void Zactor__Setshootintoground(ZREF rActor, bool enabled)
    {
        // TODO: Finish me
    }

    void Zactor__Setshootintogroundinfullbody(ZREF rActor, bool enabled)
    {
        // TODO: Finish me
    }

    float Zactor__Getboidspeed(ZREF rActor)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void Zactor__Inactivateboid(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Placeactorinmovinggroup(ZREF rActor, ZREF rGroup, ZREF rReference)
    {
        // TODO: Finish me
    }

    void Zactor__Removeactorfrommovinggroup(ZREF rActor, ZREF rGroup)
    {
        // TODO: Finish me
    }

    int Zactor__Getnumoftransitions(ZREF rActor, eMoveSet moveSet)
    {
        // TODO: Finish me
        return 0;
    }

    void Zactor__Setnumoftransitions(ZREF rActor, eMoveSet moveSet, int numTransitions)
    {
        // TODO: Finish me
    }

    void Zactor__Setfootwearmaterial(ZREF rActor, int material)
    {
        // TODO: Finish me
    }

    void Zactor__Setmovepoolweight(ZREF rActor, float weight)
    {
        // TODO: Finish me
    }

    void Zactor__Sethandtarget(ZREF rActor, int hand, ZREF rTarget, float weight)
    {
        // TODO: Finish me
    }

    int Zactor__Getvariant(ZREF rActor)
    {
        // TODO: Finish me
        return 0;
    }

    void Zactor__Setvariant(ZREF rActor, int variant)
    {
        // TODO: Finish me
    }

    void Zactor__Addtoforcedrawoffscreenlist(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Removefromforcedrawoffscreenlist(ZREF rActor)
    {
        // TODO: Finish me
    }

    void Zactor__Turnactor(ZREF rActor, float angle)
    {
        // TODO: Finish me
    }

    void Zactor__Setcanopendoors(ZREF rActor, bool canOpenDoors)
    {
        // TODO: Finish me
    }
}
