#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SUpdateLightDataSSE // Layout confirmed by Mini Ninjas XBOX PDB and PC ZRenderSoftwareLight::CalcLight
    {
        float vPosX[4];
        float vPosY[4];
        float vPosZ[4];
        float vDirXAX[4];
        float vDirXAY[4];
        float vDirXAZ[4];
        float vDirYAX[4];
        float vDirYAY[4];
        float vDirYAZ[4];
        float vDirZAX[4];
        float vDirZAY[4];
        float vDirZAZ[4];
        float cRed1[4];
        float cGreen1[4];
        float cBlue1[4];
        float cRed2[4];
        float cGreen2[4];
        float cBlue2[4];
        float fScaleX[4];
        float fScaleY[4];
        float hotpct[4];
        float ihotpct[4];
        float fFarRange[4];
        float fInvFarNear[4];
        float fCosFallOff[4];
        float fLConst1[4];
    };
    
    // Layout confirmed by Mini Ninjas XBOX PDB (stride 0x250).
    // NOTE: PC (Blood Money) uses stride 0x240, so the scalar tail below differs on PC (not reversed yet).
    // The SSE block, lType and lLightControl match PC exactly.
    struct SUpdateLightData
    {
        SUpdateLightDataSSE SSE;
        unsigned int lType;
        unsigned int lGeomCon;
        unsigned int lLightControl;
        unsigned int lDrawEntryId;
        float vPos[3];
        float mDir[9];
        float vColor1[3];
        float vColor2[3];
        float vShadowColor[3];
        float fNearRange;
        float fNearRange2;
        float fFarRange;
        float fFarRange2;
        float fInvFarNear;
        float fCosFallOff;
        float fCosHotSpot;
        float fFallOff;
        float fLConst1;
        float fScaleX;
        float fScaleY;
        float hotpct;
        float ihotpct;
        float fTanFallOff;
        float fTanHotSpot;
        float fAspect;
        unsigned int pad[3];
    };
}