#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    bool Physics__Releasebone(ZREF rGeom, v3 impulse, float force);
    bool Physics__Setvelocity(ZREF rGeom, float xRotation, float yRotation, float zRotation, v3 velocity);
}
