#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Debug/ZDebugInt.h>


namespace Glacier
{
    // fwds
    class ZBaseGeom;

    // Defined in KeyControlGeom.cpp (PC 0x009ACD14)
    extern ZDebugInt g_iFreeCamMode;

    // Movement/key control for the free camera. PC 0x00591C10.
    void KeyControlGeom(ZBaseGeom* pGeom, float fMultiplier);

    // Mouse-look that rotates a view direction by mouse movement. PC 0x00591A50 (PS2 MouseConVect1).
    // vDir  - in/out view direction (unit vector)
    // pOldMouse / pNewMouse - mouse position (X, Y) last / this frame
    // fSensX / fSensY - horizontal / vertical sensitivity
    void MouseConVect1(float* vDir, const float* pOldMouse, const float* pNewMouse, float fSensX, float fSensY);
}
