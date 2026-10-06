#pragma once

#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Geom/ZParticleEmitterAnimationHeader.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/ZSTL/TIMETYPE.h>

namespace Glacier
{
    class ZParticleEmitter : public ZSTDOBJ
    {
    public:
        DECLARE_GEOM_CLASS(ZParticleEmitter, 0x2000FFu);
        STATIC_CLASS_VAR(ZParticleEmitter, ZMessageResolver, s_msgDeactivate);
        STATIC_CLASS_VAR(ZParticleEmitter, ZMessageResolver, s_msgExplosionSplash);

        ZParticleEmitter(const char* psName, ZBaseGeom* pBaseGeom);
        ~ZParticleEmitter() override;

        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const ZGEOM* pSource) override;
        void ClassInit() override;
        void ClassInit2() override;
        void ClassFrameUpdate() override;
        int32_t ClassCommand(ZMSGID msg, void* pData) override;

        static const RTP::ZPropertyInfo& Properties();
        static uint32_t GetClassId();
        static void GetClassIdAndMask(uint32_t& id, uint32_t& mask);
        void SetSpeed(float* pSpeed);
        const float* Speed() const;
        bool GetActive() const;

        // RTP accessors do not trigger activation, hiding, or lighting updates.
        void GetbGlobal(bool& value);
        void GetbHide(bool& value);
        void GetbLighting(bool& value);
        void GetbAutoStart(bool& value);
        void GetbDeactivate(bool& value);
        void SetbGlobal(const bool& value);
        void SetbHide(const bool& value);
        void SetbLighting(const bool& value);
        void SetbAutoStart(const bool& value);
        void SetbDeactivate(const bool& value);
        void GetszParticleTemplateName(ZRTString& value);
        void SetszParticleTemplateName(const ZRTString& value);

        // PC layout: unlike PS2, both runtime clocks are fixed-point TIMETYPEs,
        // and there is no debug-box member between the flags and the clocks.
        float m_fLoopActivate;                 // +0x10
        ZVector3 m_vRandomPosition;            // +0x14
        int16_t m_lNumParticlesPerEmit;        // +0x20
        RE_ADD_PADDING(2);
        ZVector3 m_vRandomRotation;            // +0x24
        float m_fInterval;                     // +0x30
        float m_fDeactivate;                   // +0x34
        float m_fInnerConeAngle;               // +0x38
        float m_fOuterConeAngle;               // +0x3C
        ZVector3 m_vSpeed;                     // +0x40
        float m_fSpeedVariation;               // +0x4C
        float m_fSpeedInherit;                 // +0x50
        float m_fStartDelay;                   // +0x54
        ZREF m_rParticleTemplate;              // +0x58
        uint32_t m_iAnimOffsetEPS;             // +0x5C
        uint32_t m_iAnimOffsetSpeed;           // +0x60
        bool m_bLoop;                         // +0x64
        RE_ADD_PADDING(3);
        float m_fLoopInterval;                 // +0x68
        int32_t m_iLoopCount;                  // +0x6C
        bool m_bExpandBounds;                 // +0x70
        bool m_bSquareSpread;                 // +0x71
        ZMSGID m_msgActivate;                  // +0x72
        bool m_bGlobal : 1;                    // +0x74, 0x01
        bool m_bHide : 1;                      // +0x74, 0x02
        bool m_bLighting : 1;                  // +0x74, 0x04
        bool m_bAutoStart : 1;                 // +0x74, 0x08
        bool m_bDeactivate : 1;                // +0x74, 0x10
        bool m_bActive : 1;                    // +0x74, 0x20
        bool m_bEmitted : 1;                   // +0x74, 0x40 (PC-only runtime flag)
        RE_ADD_PADDING(3);
        TIMETYPE m_fActivationTime;            // +0x78
        TIMETYPE m_fEmitterTime;               // +0x7C
        ZVector3 m_vExplosionSplash;           // +0x80
        ZREF m_rParticleController;            // +0x8C
        uint32_t m_lParticleControllerIndex;   // +0x90
        ZVector3 m_vOldParticlePos;            // +0x94
        float m_fTotalLifeTime;                // +0xA0

    protected:
        bool PostLoad(ISerializerStream& stream) override;
        void UpdateAnimatedParameters();
        bool UpdateAnimatedParameterFloat(int offset, int frame, float* pValue);
        bool UpdateAnimatedParameterVector(int offset, int frame, float* pValue);

    private:
        void FinishInit();
    };
    RE_VERIFY_SIZE(ZParticleEmitter, 0xA4);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fLoopActivate, 0x10);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_vRandomPosition, 0x14);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_lNumParticlesPerEmit, 0x20);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_vRandomRotation, 0x24);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fInterval, 0x30);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fDeactivate, 0x34);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fInnerConeAngle, 0x38);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fOuterConeAngle, 0x3C);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_vSpeed, 0x40);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fSpeedVariation, 0x4C);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fSpeedInherit, 0x50);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fStartDelay, 0x54);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_rParticleTemplate, 0x58);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_iAnimOffsetEPS, 0x5C);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_iAnimOffsetSpeed, 0x60);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_bLoop, 0x64);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fLoopInterval, 0x68);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_iLoopCount, 0x6C);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_bExpandBounds, 0x70);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_bSquareSpread, 0x71);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_msgActivate, 0x72);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fActivationTime, 0x78);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fEmitterTime, 0x7C);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_vExplosionSplash, 0x80);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_rParticleController, 0x8C);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_lParticleControllerIndex, 0x90);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_vOldParticlePos, 0x94);
    RE_VERIFY_OFFSET(ZParticleEmitter, m_fTotalLifeTime, 0xA0);
}
