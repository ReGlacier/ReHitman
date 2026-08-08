#pragma once

#include <Glacier/ScriptEngine/Common.h>

namespace Glacier
{
    void Zhm3Boid__Die(ZREF rBoid);
    float Zhm3Boid__Getdistancefrombonetoobject(ZREF rBoid, int boneId, ZREF rTarget);
    v3 Zhm3Boid__Getfacing(ZREF rBoid);
    void Zhm3Boid__Disableboid(ZREF rBoid);
    void Zhm3Boid__Shootintoground(ZREF rBoid);
    void Zhm3Boid__Disableallbehaviors(ZREF rBoid);
    void Zhm3Boid__Updatelocalspace(ZREF rBoid, char updateLocalSpace);
    void Zhm3Boid__Setminmaxspeed(ZREF rBoid, float minSpeed, float maxSpeed);
    void Zhm3Boid__Resetspeed(ZREF rBoid);
    void Zhm3Boid__Setminmaxforce(ZREF rBoid, float minForce, float maxForce);
    void Zhm3Boid__Setvisionrangeandangle(ZREF rBoid, float range, float angle);
    void Zhm3Boid__Applyconstantforce(ZREF rBoid, float x, float y, float z, char enabled);
    void Zhm3Boid__Enablebanking(ZREF rBoid, char enabled);
    void Zhm3Boid__Setforceconstraint(ZREF rBoid, float x, float y, float z);
    void Zhm3Boid__Enableseparation(ZREF rBoid, float weight, int enabled);
    void Zhm3Boid__Enablecohesion(ZREF rBoid, int enabled);
    void Zhm3Boid__Enableallignment(ZREF rBoid, int enabled);
    void Zhm3Boid__Enableflee(ZREF rBoid, ZREF rFlee, float distance, int enabled);
    void Zhm3Boid__Enablefall(ZREF rBoid, float x, float y, float z, int enabled);
    void Zhm3Boid__Enablestayinbox(ZREF rBoid, ZREF rBox, int enabled);
    void Zhm3Boid__Enablestayinspherepos(ZREF rBoid, v3 position, float radius, int enabled);
    void Zhm3Boid__Enablestayinsphere(ZREF rBoid, ZREF rSphere, float radius, int enabled);
    void Zhm3Boid__Enablefollowwaypoints(ZREF rBoid, ZREF rWaypointList, float distance, int loop, int enabled);
    void Zhm3Boid__Enablewander2D(ZREF rBoid, float minX, float maxX, float minZ, float maxZ, int enabled);
    void Zhm3Boid__Enableseekref(ZREF rBoid, ZREF rTarget, int enabled);
    void Zhm3Boid__Enableseekpos(ZREF rBoid, v3 position, int enabled);
    void Zhm3Boid__Enablefallbehavior(ZREF rBoid, float x, float y, float z, int enabled);
    void Zhm3Boid__Setdragtarget(ZREF rBoid, ZREF rTarget, int boneId, v3 offset);
    void Zhm3Boid__Releasedragtarget(ZREF rBoid, char enabled);
    void Zhm3Boid__Setpos(ZREF rBoid, float x, float y, float z);
    void Zhm3Boid__Setglobalbonescale(ZREF rBoid, float scale);
}
