#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
     void Print(const char* format, ...);
     void Error(const char* format, ...);
     void Warning(const char* format, ...);
     void Zscassert(bool bCond);
    float Engine__Random();
    int Engine__Randomrange(int min, int max);
    float Engine__Gettime();
    ZREF Engine__Getsceneval(const char* name);
     void Engine__Setsceneval(const char* name, ZREF value);
     bool Engine__Localeexists(const char* name);
     bool Engine__Isvalidvector(float x, float y, float z);
}
