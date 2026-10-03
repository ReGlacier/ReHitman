#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Vehicle/ZLinkBase.h>


namespace Glacier
{
    class ZSHAPE;
    struct SHitInfo;
    struct BaseMover;
    struct BaseMoveModel;
    struct PathFinderIF;

    class ZVehicle : public ZLinkBase
    {
    public:
        // vtbl
        virtual void Init();
        virtual void OnProjectileHit(SHitInfo*);
        virtual void OnPathFinished();
        virtual void SetSpline(ZSHAPE*);
        virtual void InitRootMatPos(const ZMat3x3& mMat, const ZVector3& vPos);
        virtual void SetRootMatPos(const ZMat3x3& mMat, const ZVector3& vPos);
        virtual void GetRootMatPos(ZMat3x3& mMat, ZVector3& vPos);
        virtual float Mass(); // Always 800.h lmao
        virtual int NumSeats();
        virtual void InitSeatIndices();
        virtual uint16_t* SeatIndices();

        // data
        ZSDOwner m_SoundDef;
        ZMSGID m_msgPathFinished ;
        ZMSGID m_msgProjectileHit;
        ZMSGID m_msgDamageInfo;
        uint16_t* m_pSeatIndices;
        BaseMover* m_pMover;
        BaseMoveModel* m_pMoveModel;
        ZMat3x3 m_mRootMat;
        ZVector3 m_vRootPos;
        ZMat3x3 m_mPreRootMat;
        ZVector3 m_vPreRootPos;
        ZMat3x3 m_mInitialRootMat;
        ZVector3 m_vInitialRootPos;
        ZREF m_rPFGeom;
        PathFinderIF* m_pPFInstance;
        ZREF m_rInputPathFinder;
    };
    RE_VERIFY_SIZE(ZVehicle, 0x1D0); // verified
    RE_VERIFY_OFFSET(ZVehicle, m_pSeatIndices, 0x128); // Approved by ZVehicle::SeatIndices
}
