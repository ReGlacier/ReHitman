#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    bool Pathfinder__Isvectinside(v3 position);
    bool Pathfinder__Isposinside(float x, float y, float z);
    v3 Pathfinder__Getsearchposaroundpos(v3 position, float radius, int divisions, int index);
    v3 Pathfinder__Getposaroundpos(v3 position, float radius);
    v3 Pathfinder__Getentitypos(ZREF entity);
    v3 Pathfinder__Getclosestwallpos(ZREF actor, v3 position);
    void Pathfinder__Teleportboid(ZREF boid, v3 position);
    bool Pathfinder__Iswallbetweenpoints(v3 first, v3 second);
}
