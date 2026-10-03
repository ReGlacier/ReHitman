#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    bool Pathfollower__Ispathfollowervalid(ZREF rPathFollower);
    int Pathfollower__Numwaypointlists(ZREF rPathFollower);
    void Pathfollower__Setwaypointindex(ZREF rPathFollower, int index);
    void Pathfollower__Useclosestonstart(ZREF rPathFollower, bool useClosest);
    ZREF Pathfollower__Getnextwaypoint(ZREF rPathFollower);
    void Pathfollower__Setwaypointlist(ZREF rPathFollower, int listIndex);
    ZREF Pathfollower__Getcurrentwaypointlist(ZREF rPathFollower);
    void Pathfollower__Setexternalwaypointlist(ZREF rPathFollower, ZREF rWaypointList);
    void Pathfollower__Setpathtype(ZREF rPathFollower, eScriptPathType pathType);
    eScriptPathType Pathfollower__Getpathtype(ZREF rPathFollower);
    bool Pathfollower__Getusewalk(ZREF rPathFollower);
    bool Pathfollower__Getplayanimswhilewalking(ZREF rPathFollower);
}
