#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ReGlacier.h>

#include <Glacier/Fysix/SRigidBodyVelocity.h>
#include <Glacier/Fysix/SExplosionInfo.h>
#include <Glacier/Fysix/ZFastBoxColi.h>
#include <Glacier/Fysix/SHitInfo.h>

namespace Glacier
{
    struct ZCollisionBox;

    struct ParticleSystem
    {
        virtual ~ParticleSystem();
        virtual void Init();
    };
    RE_VERIFY_SIZE(ParticleSystem, 0x4);

    struct Particle
    {
        ZVector3 ok_x;
        ZVector3 x;
        ZVector3 oldx;
        ZVector3 v;
        float mass;
    };
    RE_VERIFY_SIZE(Particle, 0x34);

    struct ParticleConstraint
    {
        struct Particle* m_pPar1;
        struct Particle* m_pPar2;
        float m_fDist;
    };
    RE_VERIFY_SIZE(ParticleConstraint, 0xC);

    // TODO: Move to separated file
    struct ConstrainedParticleSystem : public ParticleSystem
    {
        virtual void Init() override;
        virtual ~ConstrainedParticleSystem() override;
        virtual void InitOkX();

        struct Particle* m_pParticles;                          // +0x04
        struct ParticleConstraint* m_pConstraints;              // +0x08
        struct ParticleConstraint* m_pSpecialConstraints;       // +0x0c
        int m_iNumParticles;                             // +0x10
        int m_iNumConstraints;                           // +0x14
        int m_iNumSpecialConstraints;                    // +0x18
        bool m_bFollow;                                  // +0x1c
        float m_fPrevTimeStep;                           // +0x20
        float m_fDamping;                                // +0x24
        struct ZFastBoxColi* m_pFastBox;
        bool m_bInWater;
        bool m_bReallyInWater;
        bool m_bReallyInWaterOld;
        const ZLNKOBJ* m_pLnkObj;
    };
    RE_VERIFY_SIZE(ConstrainedParticleSystem, 0x34);

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

        //vftable
        virtual void SetVelocity(const SRigidBodyVelocity& velocity);

        //api
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

        //data (total size is 0xF4, ZEventBase size is 0x30)
        uint16_t m_id;
        RE_ADD_PADDING(2);
        ConstrainedParticleSystem m_Particles;
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