#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    bool Bloodtrail__Isbloodtrail(ZREF rGeom);
    ZREF Bloodtrail__Getnexttrailpoint(ZREF rTrail);
    int Bloodtrail__Gettrailnumber(ZREF rTrail);
    void Bloodtrail__Deactivatetrail(ZREF rTrail);
}
