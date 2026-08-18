#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZREF.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZSphereImpact.h>
#include <Glacier/Fysix/ZRawStrip.h>


namespace Glacier
{
    // fwds
    struct SExtendedImpactInfo;

    struct SPushWork
    {
        float fScaledDist;
        ZVector3 vDir;
    };
    RE_VERIFY_SIZE(SPushWork, 0x10);

    struct ZCollisionBox
    {
        // methods
        ZCollisionBox();

        void SetMemBuffer(char* pBuffer, uint32_t uiSize);
        void Lock();
        void Unlock();
        bool IsLocked() const;
        void SetPushBox(const float* pfMat, const float* pfPos, const float* pfDimensions);
        void SetBox(const float* pfMat, const float* pfPos, const float* pfDimensions);
        void SetGravity(const float* pfGravity);
        void OverrideFrameTime(TIMETYPE ttFrameTime, float fDeltaTime);
        float DeltaTime() const;
        void GetStrips(uint32_t uiFlags);
        uint32_t StripCount() const;
        bool PushSphereOut(float* pfSpherePos, float* pfSphereRadius) const;
        bool PushLocalSphereOut(float* pfSpherePos, const float* pfSphereRadius) const;
        bool CheckBoxCollision(const float* pfMat, const float* pfPos, const float* pfSize) const;
        bool CheckSphereCollision(const float* pfMat, const float* pfPos, const float* pfRadius) const;
        bool CalcLineCollision(SExtendedImpactInfo* pImpactInfo, const float* pfFrom, const float* pfDir, bool bBothSides) const;
        bool CalcLineCollisionLocal(SExtendedImpactInfo* pImpactInfo, const float* pfFrom, const float* pfDir, bool bBothSides) const;
        bool CheckSphereCollisionLocal(const float* pfInvMat, const float* pfPos, const float* pfRadius) const;
        void AddActiveImpact(ZSphereImpact* pSphereImpact, ZRawStrip* pStrip, uint32_t uiImpactType, uint32_t uiTriangleNr) const;
        // TODO: Implement after full PS2 re-check (0x1BD668)
        void CalcImpactNormal(ZActiveImpactPrim* pActiveImpact, const float* pfSpherePos, const float* pfSphereRadius) const;
        void CalcOutFromImpact(float* pfSpeed, const ZSphereImpact* pSphereImpact, uint32_t uiImpactNr) const;
        void CalcOutFromImpact(float* pfSpeed, const ZSphereImpact* pSphereImpact, uint32_t uiImpactNr1, uint32_t uiImpactNr2) const;
        bool CheckSpeedAllowed(const float* pfSpeed, const ZSphereImpact* pSphereImpact, uint32_t uiImpactNr) const;
        void CalcAllowedOutSpeed(float* pfSpeed, ZSphereImpact* pSphereImpact, uint32_t uiImpactNr1, uint32_t uiImpactNr2) const;
        // TODO: Implement after CheckCutInside1 will be reversed
        void CleanupActiveImpacts(const float* pfSpherePos, const float* pfSphereRadius, ZSphereImpact* pSphereImpact) const;
        void CalcSpeedAndActiveImpacts(float* pfSpeed, const float* pfSpherePos, const float* pfSphereRadius, ZSphereImpact* pSphereImpact) const;
        bool PushSphereOutOfMesh(ZSphereImpact* pSphereImpact, float* pfSpherePos, const float* pfSphereRadius, SPushWork* pPushWork) const;
        bool PushSphereOutOfStrips(ZSphereImpact* pSphereImpact, float* pfSpherePos, const float* pfSphereRadius, SPushWork* pPushWork) const;
        uint32_t PushSphereOutOfTriangle(ZSphereImpact* pSphereImpact, float (*pfTriangle)[3], float* pfSpherePos, float fSphereRadius, SPushWork* pPushWork, uint32_t uiMaterialId, float fStairAngle) const;
        bool MovingSphereCollision(ZSphereImpact* pSphereImpact, const float* pfOldPos, float* pfNewPos, const float* pfSphereRadius, float* pfSpeed) const;
        ZBaseGeom* GetDynamicContactGeom(ZSphereImpact* pSphereImpact, const ZBaseGeom* pLastGeom) const;
        bool Solve3x3System(float* pfMat, float* pfVec, float* pfOut) const;
        void RandomUnitVector(float* pfOut) const;
        uint32_t DistPointLineVar2(const float* pfPoint, const float* pfLineA, const float* pfLineB, float* pfT, float* pfMinDist, float* pfDist, float* pfDir, uint32_t uiEdgeFlagA, uint32_t uiEdgeFlagB, uint32_t uiLineFlag) const;
        uint32_t CollideSphereAndTriangle(float (*pfTriangle)[3], float* pfSpherePos, float fSphereRadius, float* pfPushDir, float& fPushDist) const;
        void GetRootPoint(float* pfPoint) const;
        void GetLocalPoint(float* pfPoint) const;
        void GetRootPoint(float* pfOut, const float* pfPoint) const;
        void GetLocalPoint(float* pfOut, const float* pfPoint) const;
        void GetRootVect(float* pfVect) const;
        void GetRootVect(float* pfOut, const float* pfVect) const;
        void GetLocalVect(float* pfOut, const float* pfVect) const;

    protected:
        void DebugInit();

    public:
        // members
        ZREF m_rContactGeom;
        uint32_t m_lNrStrips;
        char* m_pMemBuffer;
        uint32_t m_lMemBufferSize;
        ZMat3x3 m_mBoxMatrix;
        ZVector3 m_vBoxPosition;
        ZVector3 m_vBoxDimensions;
        TIMETYPE m_fFrameTime;
        float m_fDeltaFrameTime;
        ZMat3x3 m_mPushBoxMatrix;
        ZVector3 m_vPushBoxPosition;
        ZVector3 m_vPushBoxDimensions;
        bool m_bPushBoxEnabled;
        bool m_bLocked;
    };
    RE_VERIFY_SIZE(ZCollisionBox, 0x94); // PC layout (approved by PC allocation size 0x20B4 for ZCollisionBase)
    RE_VERIFY_OFFSET(ZCollisionBox, m_bPushBoxEnabled, 0x90); // PC verified ZCollisionBox::Lock
    RE_VERIFY_OFFSET(ZCollisionBox, m_bLocked, 0x91); // PC verified ZCollisionBox::Lock, ZCollisionBox::Unlock
}
