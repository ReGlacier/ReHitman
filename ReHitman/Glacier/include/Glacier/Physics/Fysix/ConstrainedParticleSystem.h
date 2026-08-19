#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct ParticleSystem
    {
        // vtbl
        virtual ~ParticleSystem() = default;
        virtual void Init() {}
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
        // vtbl
        ~ConstrainedParticleSystem() override;
        virtual void InitOkX();
        
        // methods
        ConstrainedParticleSystem(int iType, int nMaxNumPartices);

        void LoadSave(ISerializerStream& stream, bool bSaving);
        void LoadSaveParticles(ISerializerStream& stream, bool bSaving);
        void LoadSaveConstraints(ISerializerStream& stream, bool bSaving);

        void SetNumConstraints(int n);
        void SetParticleOldPos(int i, ZVector3& oldpos);
        void GetParticlePos(int i, ZVector3& pos);
        void GetParticleValues(int i, ZVector3& pos, ZVector3& v, float& mass);
        void GetParticleValues(int i, ZVector3& pos, ZVector3& v, float& mass, ZVector3& oldpos);
        void SetParticleValues(int i, const ZVector3& pos, const ZVector3& v, float mass, const ZVector3& oldpos);
        void SetParticleVel(int i, const ZVector3& vel);
        void InitParticle(int i, const ZVector3& x, const ZVector3& v, float mass);
        void InitConstraint(int i, int ix1, int ix2);
        void BlowBomb(const ZVector3& pos, float fForce0);
        void MoveRigidBody(float fTimeStep);
        void ProjectConstraints2(int iters);

        // members
        Particle* m_pParticles;                          // +0x04
        ParticleConstraint* m_pConstraints;              // +0x08
        ParticleConstraint* m_pSpecialConstraints;       // +0x0c
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
}