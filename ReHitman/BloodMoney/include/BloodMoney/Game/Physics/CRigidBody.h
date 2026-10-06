#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Materials/ZTypedef.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/Serializer/ZSerializable.h>

#include <Glacier/Physics/Fysix/ConstrainedParticleSystem.h>
#include <Glacier/Physics/SRigidBodyVelocity.h>
#include <Glacier/Physics/SExplosionInfo.h>
#include <Glacier/Physics/ZFastBoxColi.h>
#include <Glacier/Physics/SHitInfo.h>

namespace Glacier
{
    // fwds
    struct ZCollisionBox;
    class ZBaseGeom;
    class ZGROUP;
    template <typename T> struct ZArray;
}

namespace Hitman
{
    struct SVertexRep
    {
        float c[4];
        float fQuadSum;
    };
    RE_VERIFY_SIZE(SVertexRep, 0x14);

    // Reversed from `engine/fysix/rigidbody.cpp`. It lives in the BloodMoney module (namespace
    // `Hitman`, like `Glacier::ZCloth` before it) because it references HM3-only game types
    // (`Hitman::ZHM3GameData`, `ZHM3Actor`) that the Glacier module cannot depend on.
    class CRigidBody : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        //data
        static constexpr const char* Name = "QRigidBody";
        static constexpr const char* ClassName = "ZGEOM_QRigidBody";
        static constexpr uint32_t kMaxVertices = 500; // PC: GetVertices/AddVertex cap

        // RTTI
        DECLARE_ROUT_CLASS(CRigidBody, ZBaseConRout, CRigidBody, 0, 0);

        // static
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgProjectileHit);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgExplodeBomb);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgIKBomb_Explode);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgActivate);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgFreeze);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgThaw);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgSetPos);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgSetVelocity);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgSetMaterial);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgSendImpactEvent);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgCollision);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgRemove);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgRequestDeltaY);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgGetElevBound);
        STATIC_CLASS_VAR(CRigidBody, Glacier::ZMessageResolver, m_msgGetHatchGeom);
        STATIC_CLASS_VAR(CRigidBody, Glacier::TAudioPropertyID, m_MaterialProperty_SoundMaterial);

        // ctor / dtor
        CRigidBody();
        ~CRigidBody() override;

        // vtbl
        // RTP::cBase
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        static const Glacier::RTP::ZPropertyInfo& Properties();
        // ZEventBase
        void Init() override;
        void End() override;
        void CopyData(const Glacier::ZEventBase* source) override;
        void FrameUpdate() override;
        int32_t Command(Glacier::ZMSGID command, Glacier::ZDATA data) override;
        // ZSerializable
        bool PostLoad(Glacier::ISerializerStream& stream) override;
        // CRigidBody
        virtual void SetVelocity(const Glacier::SRigidBodyVelocity& velocity);

        // methods (non-virtual)
        bool Activated();
        void GetRemoveAfterUse(bool& bRemoveAfterUse);
        void SetRemoveAfterUse(const bool& bRemoveAfterUse);
        void GetFrozen(bool& bFrozen);
        void SetFrozen(const bool& bFrozen);

        void SetVelocity(const Glacier::SRigidBodyVelocity* velocity);

        void Enable();
        void Disable();
        void DisableRemove(bool);
        void SetPos(const Glacier::ZVector3* position);
        void SetupTransform();
        void HandleHit(Glacier::SHitInfo* hitInfo);
        void HandleExplodeBomb(Glacier::SExplosionInfo* explosionInfo);
        void PlaySound();
        void CheckCollision4a(Glacier::ZCollisionBox* collisionBox);
        void CheckCollision4b(Glacier::ZCollisionBox* collisionBox);
        void Setup();
        void Setup2();

    public:
        bool AddVertex(Glacier::ZVector3* pVertices, const Glacier::ZVector3* pPoint, uint32_t& lCount, float fMinDistSq);
        void AddBBox(Glacier::ZVector3* pVertices, uint32_t& lCount, const float* pfCen, const float* pfSize);
        bool AddVertices(const Glacier::ZBaseGeom* pBaseGeom, Glacier::ZVector3* pVertices, uint32_t& lCount);
        uint32_t GetVertices(Glacier::ZVector3* pVertices, bool bForced);
        void InitCentroid(Glacier::ZVector3* pVertices);
        void InitParticles();
        void InitVertexReps(Glacier::ZVector3* pVertices);
        float KineticEnergy(Glacier::ZVector3* pOutCentroid);
        void SetMatPosFromTetrahedron();
        void Tetrahedron(Glacier::ZVector3& v0, Glacier::ZVector3& v1, Glacier::ZVector3& v2, Glacier::ZVector3& v3);
        float Move(float fTimeStep);

        void Impact(const Glacier::ZVector3& point, const Glacier::ZVector3& direction);
        void ImpactImpl(const Glacier::ZVector3& direction, const float* pWeights, int a4);
        void AdjustFace2(Glacier::ZVector3& x0, Glacier::ZVector3& v0, float mass0,
                         Glacier::ZVector3& x1, Glacier::ZVector3& v1, float mass1,
                         Glacier::ZVector3& x2, Glacier::ZVector3& v2, float mass2,
                         Glacier::ZVector3& x3, Glacier::ZVector3& v3, float mass3,
                         const Glacier::ZVector3& shapePos, const Glacier::ZVector3& contactPos,
                         float w0, float w1, float w2, float w3, float quadSum);

        void UpdateElevatorState();
        float GetElevatorDeltaY();
        void AttachToElevator(uint32_t rElevator);
        void DetachFromElevator();

    public:
        // members
        uint16_t m_id;
        RE_ADD_PADDING(2);
        Glacier::ConstrainedParticleSystem m_Particles { 3, 5 };
        Glacier::ZVector3 m_Centroid;
        Glacier::ZVector3 m_LocalCentroid;
        Glacier::ZMat3x3 m_QMat;
        Glacier::ZVector3 m_QPos;
        SVertexRep* m_pVertexReps;
        uint32_t m_nNumVertices;
        Glacier::ZBaseGeom* m_pHitAnything;
        uint32_t m_iColiMaterialDescId;
        Glacier::ZVector3 m_OldPos;
        uint16_t m_nTimeOut;
        uint16_t m_iStopCount;
        float m_fWeightedSpeed;
        float m_fThreshold;
        float m_fLastNrg;
        Glacier::TIMETYPE m_fLastHitTime;
        uint16_t m_iImpactNum;
        uint32_t m_rMaterial;
        uint8_t m_Status;
        uint32_t m_rContainingElevator;
        bool m_bDoNotAddSoundEvent;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(CRigidBody, 0xF4); // Verified
}
