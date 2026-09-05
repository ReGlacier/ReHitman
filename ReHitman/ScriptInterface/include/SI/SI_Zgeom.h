#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    v3 Zgeom__Getsize(ZREF rGeom);
    void Zgeom__Setposition(ZREF rGeom, float x, float y, float z);
    v3 Zgeom__Getposition(ZREF rGeom);
    void Zgeom__Getlocalpoint(ZREF rGeom, v3& point);
    void Zgeom__Getposdir(ZREF rGeom, v3& position, v3& direction);
    void Zgeom__Setposdir(ZREF rGeom, v3& position, v3& direction);
    bool Zgeom__Getclosestposdirinbox(ZREF rBox, ZREF rGeom, ZREF rTarget, float radius, v3& position, v3& direction);
    float Zgeom__Getdistancetoobject(ZREF rGeom, ZREF rTarget);
    float Zgeom__Getdistancetopos(ZREF rGeom, v3 position);
    bool Zgeom__Hascontroller(ZREF rGeom, const char* controllerName);
    ZREF Zgeom__Getcontroller(ZREF rGeom, const char* controllerName);
    float Zgeom__Getangletoobject(ZREF rGeom, ZREF rTarget);
    float Zgeom__Getangletodir(ZREF rGeom, v3 direction);
    float Zgeom__Getangletogeomdir(ZREF rGeom, ZREF rTarget);
    void Zgeom__Copyobjectposdir(ZREF rGeom, ZREF rTarget);
    void Zgeom__Copyobjectpos(ZREF rGeom, ZREF rTarget);
    void Zgeom__Copyobjectdir(ZREF rGeom, ZREF rTarget);
    v3 Zgeom__Getobjectrelpos(ZREF rGeom, float x, float y, float z);
    bool Zgeom__Checkworldpointinside(ZREF rGeom, v3 worldPosition);
    bool Zgeom__Isinview(ZREF rGeom);
    void Zgeom__Printname(ZREF rGeom);
    float Zgeom__Getdistancetoitem(ZREF rGeom, ZREF rItem);
    void Zgeom__Makeinactive(ZREF rGeom);
    void Zgeom__Makeactive(ZREF rGeom);
    void Zgeom__Hide(ZREF rGeom, bool bHide);
    ZREF Zgeom__Getcurrentroom(ZREF rGeom);
    int Zgeom__Numberofexitsfromgeomscurrentroom(ZREF _, ZREF rGeom);
}
