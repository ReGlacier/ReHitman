#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    bool Istring__Isnameequal(ZREF geom, const char* name);
    bool Istring__Isnamecontains(ZREF geom, const char* name);
    bool Istring__Isempty(const char* value);
}
