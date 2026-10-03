#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Addlinedecay(v3 start, v3 end, int color, float decay);
    void Addvectordecay(v3 start, v3 end, float decay, int color, float arrowLength);
    void Debugfunctions__Displayscriptline(ZREF firstGeom, ZREF secondGeom, int color);
    void Debugfunctions__Displayscriptlinefrompos(v3 start, v3 end, int color);
    int Debugfunctions__Displaypermanentscriptline(ZREF firstGeom, ZREF secondGeom, int color);
    int Debugfunctions__Displaypermanentscriptlinefrompos(v3 start, v3 end, int color);
    bool Debugfunctions__Modifypermanentscriptline(int line, ZREF firstGeom, ZREF secondGeom, int color);
    int Debugfunctions__Addpermanentscripttext(ZREF geom, int height, int color, const char* format, ...);
}
