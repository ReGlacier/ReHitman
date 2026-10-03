#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    int Audio__Playsound2D(int soundId);
    int Audio__Playsound3D(int soundId, ZREF object);
    int Audio__Playlocaleresource2D(const char* name);
    int Audio__Getlocaleresource(const char* name);
    void Audio__Setnotifytarget(ZREF target, int soundObject);
    float Audio__Gettimeplayed(ZREF soundObject);
    float Audio__Gettimeleft(ZREF soundObject);
    void Audio__Stopsound(ZREF soundObject);
    void Audio__Setvolume(ZREF soundObject, float volume);
    ZREF Audio__Getsndobjsoundobjectref(ZREF object);
    void Audio__Setsoundobjectautodelete(ZREF soundObject, bool autoDelete);
}
