#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Zusepoint__Lock(ZREF rUsePoint);
    void Zusepoint__Unlock(ZREF rUsePoint);
    void Zusepoint__Changeprobability(ZREF rUsePoint, float probability);
    void Zusepoint__Relocate(ZREF rUsePoint);
    bool Zusepoint__Islocked(ZREF rUsePoint);
}
