#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/ZMessageResolver.h>

namespace Glacier
{
    class ZCloth;

    struct ClothPositionPtr
    {
        float* m_pPositions;
        uint32_t m_Prim;
        ZCloth* m_pOwner;
        float m_LastUsedTime;
    };

    class ZCloth : public ZSTDOBJ
    {
    public:
        enum ActorCollisionType : uint32_t { NONE = 0, HERO = 1, ALL = 2 };
        enum WindType : uint32_t { NO = 0, STATIC = 1, PERLIN = 2, MOVEMENTBASED = 3 };

        struct ClothConstraint
        {
            uint16_t ix1;
            uint16_t ix2;
            float m_fWeightedInvMassSum;
            float m_fSquare;
            float m_fDoubleSquare;
        };

        DECLARE_GEOM_CLASS(ZCloth, 0x2000F6u);
        STATIC_CLASS_VAR(ZCloth, ZMessageResolver, m_msgChangeWindSpeed);
        STATIC_CLASS_VAR(ZCloth, ZMessageResolver, m_msgChangeElasticity);

        ~ZCloth() override;
        bool PostLoad(ISerializerStream& stream) override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void CorrectOwnerDrawMatrix(ZMat3x3&, ZVector3&, ZBaseGeom*, uint32_t) override;
        bool WantDrawBufferControl() const override;
        void DrawUpdate() override;
        bool DrawBufferViewUpdate(ZDrawBuffer*, ZCameraSpace*) override;
        bool DrawBufferViewUpdate(ZDrawBuffer*, ZBaseGeom*, uint32_t) override;
        void ClassInit() override;
        int32_t ClassCommand(ZMSGID, void*) override;
        void DrawIt(const ZMat3x3& objectToWorld, const ZVector3& position);
        virtual void HandleTensions(float* pParticles);
        virtual void Initialize(bool initializeProperties);

        ZCloth(const char* psName, ZBaseGeom* pBaseGeom);

        ZPrimHandle* m_ppPrimAccess;
        uint32_t m_iIterations;
        uint32_t m_iMaxNrVisible;
        ActorCollisionType m_iActorCollision;
        WindType m_iUseWind;
        ZVector3 m_WindSpeed;
        float m_fElasticity;
        float m_fGravity;
        float m_fDamping;
        uint32_t m_lNrClothPrims;
        uint32_t m_lNrParticles;
        uint32_t m_lNrConstraints;
        ClothConstraint* m_pConstraints;
        uint16_t* m_pCopyList;
        uint32_t m_lCopyListLen;
        float* m_pParticles;
        ClothPositionPtr* m_ppPositions;
        ClothPositionPtr* m_pCurClothPositionPtr;
        float* m_pParticlesMass;
        float* m_pParticlesInvMass;
        uint32_t* m_pPrimIndex;
        uint32_t m_lNrTriangles;
        uint16_t* m_pTriangles;
        uint32_t m_lnrBoxes;
        ZGEOM** m_pBoxes;
        ZREF m_rHero;
        ZGEOM* m_pHero;
        float* m_pGroupActiveTime;
        uint32_t m_iOrgPrim;
        uint32_t m_lNrCollisionPlanes;
        uint32_t m_lNrCollisionPlanesInAll;
        ZBaseGeom** m_pCollisionPlanes;
        void* m_pCollisionPlanes2;
        ZVector3 m_OldPosition;
        float m_fSyncTimeStep;
        uint8_t* m_pDataBlockPtr;
    };

    RE_VERIFY_SIZE(ZCloth::ClothConstraint, 0x10);
    RE_VERIFY_SIZE(ClothPositionPtr, 0x10);
    RE_VERIFY_SIZE(ZCloth, 0xB0);
    RE_VERIFY_OFFSET(ZCloth, m_ppPrimAccess, 0x10);
    RE_VERIFY_OFFSET(ZCloth, m_pBoxes, 0x78);
    RE_VERIFY_OFFSET(ZCloth, m_OldPosition, 0x9C);
    RE_VERIFY_OFFSET(ZCloth, m_pDataBlockPtr, 0xAC);
}
