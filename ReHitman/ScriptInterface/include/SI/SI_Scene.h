#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Scene__Setobjectposition(ZREF rObject, float x, float y, float z);
    void Scene__Setobjectpositionv3(ZREF rObject, v3 position);
    v3 Scene__Getobjectposition(ZREF rObject);
    int Scene__Getgeomsinsidebox(ZREF rBox, ZREF* outGeoms, int maxGeoms, bool includeInactive, bool includeChildren);
    int Scene__Getactorsinbox(ZREF rBox, ZREF* outActors, int maxActors);
    bool Scene__Isbox(ZREF rObject);
    bool Scene__Isposinsidebox(ZREF rBox, v3 position);
    v3 Scene__Getposinbox(ZREF rBox);
    void Scene__Setscale(ZREF rObject, float x, float y, float z);
    bool Scene__Objecthascontroller(ZREF rObject, const char* controllerName);
    ZREF Scene__Getcontrolleronobject(ZREF rObject, const char* controllerName);
    float Scene__Getdistancebetweenobjects(ZREF firstObject, ZREF secondObject);
    float Scene__Getdistbetweenobjectandpos(ZREF rObject, v3 position);
    float Scene__Getdistbetweenobjectandpos2D(ZREF rObject, v3 position);
    int Scene__Getnumofaccessdenies(ZREF rObject, int accessType);
    void Scene__Makeinactive(ZREF rObject);
    void Scene__Makeactive(ZREF rObject);
    int Scene__Getdoorstatus(ZREF rObject);
    ZREF Scene__Getparent(ZREF rObject);
    bool Scene__Isshattered(ZREF rObject);
    bool Scene__Isground(ZREF rObject);
    bool Scene__Setnoiselevel(ZREF rObject, int level);
    int Scene__Getnoiselevel(ZREF rObject);
    bool Scene__Getisinoutsidelocation(ZREF rObject);
}
