#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    class ZParticleTemplate : public ZSTDOBJ
    {
    public:
        DECLARE_GEOM_CLASS(ZParticleTemplate, 0x2000E5u);

        ~ZParticleTemplate() override;

        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void ClassInit2() override;
        ZGEOM* DuplicateInit(ZGROUP* pDestGroup, const ZMat3x3* pMat, const ZVector3* pPos, const char* psDupName, bool bRecursive) override;
        void CopyData(const ZGEOM* pSource) override;

        ZParticleTemplate(const char* psName, ZBaseGeom* pBaseGeom);

        uint32_t ParticlePrim() const { return m_lPrimsTable; }
        uint32_t ControllerIndex() const { return m_lControllerIndex; }
        void GetController(ZREF* pController, uint32_t* pIndex);
        ZREF GetNextTemplate() const { return m_rNextTemplate; }
        const ZVector3& GetGravity() const { return m_vGravity; }
        const ZVector3& GetScaledGravity() const { return m_vScaledGravity; }
        float GetFriction() const { return m_fFriction; }
        float GetMaxAge() const { return m_fMaxAge; }
        float GetAgeVariation() const { return m_fAgeVariation; }
        const uint32_t* GetEffectColorTable() const { return m_pColorTable; }
        float GetScale() const { return m_fScale; }
        float GetScaleVariation() const { return m_fScaleVariation; }
        float GetScaleVel() const { return m_fScaleVel; }
        float GetScaleAcc() const { return m_fScaleAcc; }
        float GetAngleSpeed() const { return m_fAngleSpeed; }
        float GetAngleSpeedVel() const { return m_fAngleSpeedVel; }
        float GetAngleSpeedAcc() const { return m_fAngleSpeedAcc; }
        bool GetRandomStartAngle() const { return m_bRandomStartAngle; }
        bool GetAlignWithEmitterDir() const { return m_bAlignWithEmitterDir; }
        bool GetAlignWithParticleDir() const { return m_bAlignWithParticleDir; }
        float GetAngleStart() const { return m_fAngleStart; }
        float GetMotionStretch() const { return m_fMotionStretch; }
        const ZVector3& GetWindSpeed() const { return m_vWindSpeed; }
        float GetWindForce() const { return m_fWindForce; }
        float GetCollisionRate() const { return m_fCollisionRate; }
        float GetRestitutionFactor() const { return m_fRestitutionFactor; }
        int32_t GetColorRepeat() const { return m_lColorRepeat; }
        float GetChanceTransfer() const { return m_fChanceTransfer; }
        const float* GetColorKeys() const { return &m_vColorKeys[0][0]; }
        bool GetFake3dRotation() const { return m_bFake3dRotation; }
        bool GetRandomTrajectoryEnvelope() const { return m_bRandomTrajectoryEnvelope; }
        bool GetRandomRotationDir() const { return m_bRandomRotationDir; }

        static ZREF FindTemplate(const char* psName);

        ZVector3 m_vGravity;
        float m_fFriction;
        float m_fMaxAge;
        float m_fAgeVariation;
        float m_fScale;
        float m_fScaleVariation;
        float m_fScaleVel;
        float m_fScaleAcc;
        float m_fAngleSpeed;
        float m_fAngleSpeedVel;
        float m_fAngleSpeedAcc;
        bool m_bRandomStartAngle;
        bool m_bAlignWithEmitterDir;
        bool m_bAlignWithParticleDir;
        uint8_t m_pad47;
        float m_fAngleStart;
        float m_fMotionStretch;
        ZVector3 m_vWindSpeed;
        float m_fWindForce;
        float m_fCollisionRate;
        float m_fRestitutionFactor;
        int32_t m_lColorRepeat;
        bool m_bFake3dRotation;
        bool m_bRandomTrajectoryEnvelope;
        bool m_bRandomRotationDir;
        uint8_t m_pad6F;
        ZREF m_rNextTemplate;
        uint32_t m_lPrimsTable;
        ZFILENAME m_pszFileName;
        ZRTString m_szNextTemplateName;
        float m_fChanceTransfer;
        float m_vColorKeys[8][4];
        uint32_t* m_pColorTable;
        ZVector3 m_vScaledGravity;
        uint32_t m_lControllerIndex;

    private:
        void CreateColorTable();
    };
    RE_VERIFY_SIZE(ZParticleTemplate, 0x118);
    RE_VERIFY_OFFSET(ZParticleTemplate, m_vGravity, 0x10);
    RE_VERIFY_OFFSET(ZParticleTemplate, m_pColorTable, 0x104);
    RE_VERIFY_OFFSET(ZParticleTemplate, m_lControllerIndex, 0x114);
}
