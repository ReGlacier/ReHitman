#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Poscontroller__Addactor(ZREF rPosController, ZREF rActor);
    void Poscontroller__Removeactor(ZREF rPosController, ZREF rActor);
    v3 Poscontroller__Getposition(ZREF rPosController, ZREF rActor);
}
