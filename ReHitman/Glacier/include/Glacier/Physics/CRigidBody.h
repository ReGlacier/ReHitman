#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ReGlacier.h>

#include <Glacier/Physics/Fysix/ConstrainedParticleSystem.h>
#include <Glacier/Physics/SRigidBodyVelocity.h>
#include <Glacier/Physics/SExplosionInfo.h>
#include <Glacier/Physics/ZFastBoxColi.h>
#include <Glacier/Physics/SHitInfo.h>

namespace Glacier
{
    struct ZCollisionBox;

    struct SVertexRep
    {
        float c[4];
        float fQuadSum;
    };
    RE_VERIFY_SIZE(SVertexRep, 0x14);

    class CRigidBody : public CBaseEvent<ZGEOM>
    {
    public:
        //data
        static constexpr const char* Name = "QRigidBody";
        static constexpr const char* ClassName = "ZGEOM_QRigidBody";

        // vtbl
        // RTP::cBase
        // ZEventBase
        // ZBaseConRout
        // CRigidBody
        virtual void SetVelocity(const SRigidBodyVelocity& velocity);

        // methods
        void Enable();
        void Disable();
        void DisableRemove(bool);
        void SetPos(const ZVector3* position);
        void SetVelocity(const SRigidBodyVelocity* velocity);
        void SetupTransform();
        void HandleHit(SHitInfo* hitInfo);
        void HandleExplodeBomb(SExplosionInfo* explosionInfo);
        void PlaySound();
        void CheckCollision4a(ZCollisionBox* collisionBox);
        void CheckCollision4b(ZCollisionBox* collisionBox);

        // members
        uint16_t m_id;
        RE_ADD_PADDING(2);
        ConstrainedParticleSystem m_Particles { 3, 5 };
        ZVector3 m_Centroid;
        ZVector3 m_LocalCentroid;
        ZMat3x3 m_QMat;
        ZVector3 m_QPos;
        SVertexRep* m_pVertexReps;
        uint32_t m_nNumVertices;
        ZBaseGeom* m_pHitAnything;
        uint32_t m_iColiMaterialDescId;
        ZVector3 m_OldPos;
        uint16_t m_nTimeOut;
        uint16_t m_iStopCount;
        float m_fWeightedSpeed;
        float m_fThreshold;
        float m_fLastNrg;
        TIMETYPE m_fLastHitTime;
        uint16_t m_iImpactNum;
        uint32_t m_rMaterial;
        uint8_t m_Status;
        uint32_t m_rContainingElevator;
        bool m_bDoNotAddSoundEvent;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(CRigidBody, 0xF4); // Verified
}
