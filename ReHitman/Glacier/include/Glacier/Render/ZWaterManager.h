#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SSineWave
    {
        float fP;
        float fAmplitude;
        float fQ;
        float fR;
    };
    RE_VERIFY_SIZE(SSineWave, 0x10);

    struct SWaterRippleInst
    {
        float fXCenter;
        float fYCenter;
        float fZCenter;
        float fOneOverRadius;
        float fAmplitude;
        float fRadius;
        float fFrequency;
        float fPhase;
        float fWaveLength;
        float fSpeed;
        unsigned int lGroupID;
        float fDamping;
        float fRadiusSpeed;
    };
    RE_VERIFY_SIZE(SWaterRippleInst, 0x34);

    struct SWaterPatchInst
    {
        int iXsegs;
        int iZsegs;
        float fSegSizeX;
        float fSegSizeZ;
        float fXPos;
        float fYPos;
        float fZPos;
        int iRipples;
        SSineWave pSineWaves[4];
        float pSineWavesSpeed[4];
        float pSineWavesFreq[4];
        unsigned int lGroupID;
        void *pKey;
        SWaterRippleInst *ripples[8];
    };
    RE_VERIFY_SIZE(SWaterPatchInst, 0xA8);

    class ZWaterManager
    {
    public:
        // methods
        ZWaterManager();
        ZWaterManager(int iTotalPatches);

        void InitManager(int iTotalPatches);
        SWaterPatchInst* AllocWaterPatch(void* pKey);
        void FreeWaterPatch(void* pKey);
        int UpdateRippleInfo(SWaterPatchInst* p);
        void FrameUpdate(float fTime);

        // members
        int m_iTotalPatches;
        int m_iNumPatches;
        SWaterPatchInst* m_pPatchLinearMem;
        SWaterPatchInst** m_pPatches;
        int m_iNumRipples;
        SWaterRippleInst m_pRipples[8];
    };
    RE_VERIFY_SIZE(ZWaterManager, 0x1B4); // Verified PC alloc
}
