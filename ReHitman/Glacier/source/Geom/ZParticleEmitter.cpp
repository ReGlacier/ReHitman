#include <Glacier/Geom/ZParticleEmitter.h>

#include <Glacier/Animation/StreamPacker.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZParticleController.h>
#include <Glacier/Geom/ZParticleTemplate.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZSysInterface.h>
#include <cmath>

namespace Glacier
{
    namespace
    {
        constexpr float EmitterEpsilon = 0.0001220703125f;

        bool SampleEmitterAnimation(int offset, int frame, int16_t* idToPosition, float* value)
        {
            if (offset == -1)
                return false;

            auto* data = reinterpret_cast<uint8_t*>(g_pSysInterface->m_pEngineData->GetStaticBuffer()) + offset;
            const auto* header = reinterpret_cast<const ZParticleEmitterAnimationHeader*>(data);
            const int samples = header->iFrameEnd - header->iFrameStart;
            int sample;
            if (frame >= header->iFrameEnd)
                sample = samples - 1;
            else if (frame >= header->iFrameStart)
                sample = frame - header->iFrameStart;
            else
                return false;

            StreamPacker::DePackVector(data + sizeof(*header), samples, static_cast<float>(sample),
                idToPosition, value, 0xFFFF);
            return true;
        }
    }

    ZParticleEmitter::ZParticleEmitter(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
        , m_rParticleTemplate(0)
        , m_fActivationTime()
        , m_fEmitterTime()
        , m_vExplosionSplash()
        , m_vOldParticlePos()
    {
        // Configurable fields are loaded by RTP; the PC geom allocator zeroes
        // the allocation before construction, rather than supplying editor defaults.
        BaseGeom()->SetControl(0, ZCCOLIMASK);
        m_bActive = false;
        m_bEmitted = false;
    }

    ZParticleEmitter::~ZParticleEmitter() = default;
    const RTP::ZPropertyInfo& ZParticleEmitter::GetProperties() const { return Info; }
    const RTP::ZPropertyInfo& ZParticleEmitter::Properties() { return Info; }
    uint32_t ZParticleEmitter::GetObjectId() const { return m_Id; }
    uint32_t ZParticleEmitter::GetClassId() { return m_Id; }
    void ZParticleEmitter::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const { GetClassIdAndMask(id, mask); }
    void ZParticleEmitter::GetClassIdAndMask(uint32_t& id, uint32_t& mask) { id = m_Id; mask = m_Mask; }
    ZGEOMCLASSINFO* ZParticleEmitter::GetOldClassInfo() const { return m_OldClassInfo; }
    bool ZParticleEmitter::PostLoad(ISerializerStream&) { return true; }

    void ZParticleEmitter::SetSpeed(float* pSpeed) { vcpy(m_vSpeed, pSpeed); }
    const float* ZParticleEmitter::Speed() const { return m_vSpeed; }
    bool ZParticleEmitter::GetActive() const { return m_bActive; }

    void ZParticleEmitter::GetbGlobal(bool& value) { value = m_bGlobal; }
    void ZParticleEmitter::GetbHide(bool& value) { value = m_bHide; }
    void ZParticleEmitter::GetbLighting(bool& value) { value = m_bLighting; }
    void ZParticleEmitter::GetbAutoStart(bool& value) { value = m_bAutoStart; }
    void ZParticleEmitter::GetbDeactivate(bool& value) { value = m_bDeactivate; }
    void ZParticleEmitter::SetbGlobal(const bool& value) { m_bGlobal = value; }
    void ZParticleEmitter::SetbHide(const bool& value) { m_bHide = value; }
    void ZParticleEmitter::SetbLighting(const bool& value) { m_bLighting = value; }
    void ZParticleEmitter::SetbAutoStart(const bool& value) { m_bAutoStart = value; }
    void ZParticleEmitter::SetbDeactivate(const bool& value) { m_bDeactivate = value; }

    void ZParticleEmitter::GetszParticleTemplateName(ZRTString&)
    {
        // PC registers the shared int3 accessor at 0x4636F0 for this property.
        ZASSERT(false);
    }

    void ZParticleEmitter::SetszParticleTemplateName(const ZRTString& value)
    {
        if (value.c_str() && *value.c_str() && !RefToPtr(m_rParticleTemplate))
            m_rParticleTemplate = ZParticleTemplate::FindTemplate(value.c_str());
    }

    void ZParticleEmitter::ClassInit()
    {
        CameraMessages(true);
        if (m_fInterval == m_fDeactivate)
            m_fInterval += 0.1f;
        constexpr float DegreesToRadians = 0.017453292f;
        m_fInnerConeAngle *= DegreesToRadians;
        m_fOuterConeAngle *= DegreesToRadians;
        vscalar(m_vRandomRotation, DegreesToRadians);
    }

    void ZParticleEmitter::ClassInit2()
    {
        auto* particleTemplate = geom_cast<ZParticleTemplate>(RefToPtr(m_rParticleTemplate));
        if (!particleTemplate)
            return;

        particleTemplate->GetController(&m_rParticleController, &m_lParticleControllerIndex);
        m_fTotalLifeTime = 0.0f;
        do
        {
            m_fTotalLifeTime += particleTemplate->GetMaxAge();
            particleTemplate = static_cast<ZParticleTemplate*>(RefToPtr(particleTemplate->GetNextTemplate()));
        }
        while (particleTemplate);
        FinishInit();
    }

    void ZParticleEmitter::FinishInit()
    {
        if (m_bHide)
            Hide(true);
        if (m_bAutoStart)
        {
            m_fActivationTime.secs = g_pSysInterface->FrameTime.secs - static_cast<int>(m_fStartDelay * -1024.0f);
            m_fEmitterTime = m_fActivationTime;
            m_bEmitted = false;
            m_fLoopActivate = static_cast<float>(g_pSysInterface->FrameTime);
            if (m_bGlobal)
                EnableClassCall(0x10);
            m_bActive = true;
        }
        vreset(m_vOldParticlePos);

        if (m_bExpandBounds)
        {
            ZGROUP* parent = BaseGeom()->ParentGroup();
            ZMat3x3 matrix;
            ZVector3 position, center, size;
            parent->GetRootTM(matrix, position);
            parent->GetCen(center);
            parent->GetSize(size);
            vmmul(center, matrix);
            vadd(center, position);
            SetRadius(vlen(size) + 1.0f);
            SetSize(size);
            GetRootTM(matrix, position);
            vsub(center, position);
            vmtmul(center, matrix);
            SetCen(center);
        }
    }

    void ZParticleEmitter::CopyData(const ZGEOM* pSource)
    {
        ZGEOM::CopyData(pSource);
        const auto* source = geom_cast<ZParticleEmitter>(pSource);
        if (!source)
            return;

        m_bGlobal = source->m_bGlobal;
        m_bHide = source->m_bHide;
        m_bLighting = source->m_bLighting;
        m_bAutoStart = source->m_bAutoStart;
        m_bDeactivate = source->m_bDeactivate;
        m_fInterval = source->m_fInterval;
        m_fDeactivate = source->m_fDeactivate;
        m_lNumParticlesPerEmit = source->m_lNumParticlesPerEmit;
        m_vRandomPosition = source->m_vRandomPosition;
        m_vRandomRotation = source->m_vRandomRotation;
        m_fInnerConeAngle = source->m_fInnerConeAngle;
        m_fOuterConeAngle = source->m_fOuterConeAngle;
        m_vSpeed = source->m_vSpeed;
        m_fSpeedVariation = source->m_fSpeedVariation;
        m_fSpeedInherit = source->m_fSpeedInherit;
        m_msgActivate = source->m_msgActivate;
        m_fStartDelay = source->m_fStartDelay;
        m_rParticleTemplate = source->m_rParticleTemplate;
        m_rParticleController = source->m_rParticleController;
        m_lParticleControllerIndex = source->m_lParticleControllerIndex;
        m_fActivationTime = source->m_fActivationTime;
        m_vOldParticlePos = source->m_vOldParticlePos;
        m_fTotalLifeTime = source->m_fTotalLifeTime;
        m_bExpandBounds = source->m_bExpandBounds;
        m_iAnimOffsetEPS = source->m_iAnimOffsetEPS;
        m_iAnimOffsetSpeed = source->m_iAnimOffsetSpeed;
        // PC deliberately does not copy loop settings, square spread, or runtime flags.
        FinishInit();
    }

    bool ZParticleEmitter::UpdateAnimatedParameterFloat(int offset, int frame, float* pValue)
    {
        int16_t idToPosition[] = {0};
        return SampleEmitterAnimation(offset, frame, idToPosition, pValue);
    }

    bool ZParticleEmitter::UpdateAnimatedParameterVector(int offset, int frame, float* pValue)
    {
        int16_t idToPosition[] = {0, 1, 2};
        return SampleEmitterAnimation(offset, frame, idToPosition, pValue);
    }

    void ZParticleEmitter::UpdateAnimatedParameters()
    {
        if (m_iAnimOffsetEPS == 0xFFFFFFFFu && m_iAnimOffsetSpeed == 0xFFFFFFFFu)
            return;
        const int elapsedTicks = g_pSysInterface->FrameTime.secs - m_fActivationTime.secs;
        if (elapsedTicks < 0)
            return;

        const int frame = static_cast<int>(static_cast<double>(elapsedTicks) * (25.0 / 1024.0));
        float rate;
        if (UpdateAnimatedParameterFloat(static_cast<int>(m_iAnimOffsetEPS), frame, &rate))
            m_fInterval = rate >= 0.01f ? 1.0f / rate : -1.0f;
        ZVector3 speed;
        if (UpdateAnimatedParameterVector(static_cast<int>(m_iAnimOffsetSpeed), frame, speed))
            m_vSpeed = speed;
    }

    int32_t ZParticleEmitter::ClassCommand(ZMSGID msg, void* pData)
    {
        if (msg == 2053)
        {
            if (!m_bGlobal && m_bActive)
            {
                vreset(m_vOldParticlePos);
                DisableClassCall(0x10);
            }
        }
        else if (msg == 2054)
        {
            if (!m_bGlobal && m_bActive)
                EnableClassCall(0x10);
        }
        else if (msg == m_msgActivate)
        {
            vreset(m_vOldParticlePos);
            vreset(m_vExplosionSplash);
            m_fActivationTime.secs = g_pSysInterface->FrameTime.secs - static_cast<int>(m_fStartDelay * -1024.0f);
            m_fEmitterTime = m_fActivationTime;
            m_bEmitted = false;
            m_fLoopActivate = static_cast<float>(g_pSysInterface->FrameTime);
            if (m_bGlobal || (BaseGeom()->Control() & ZCINVIEW))
                EnableClassCall(0x10);
            m_bActive = true;
        }
        else if (s_msgDeactivate == msg)
        {
            DisableClassCall(0x10);
            m_bActive = false;
        }
        else if (s_msgExplosionSplash == msg)
        {
            vcpy(m_vExplosionSplash, static_cast<const float*>(pData));
        }
        return 0;
    }

    void ZParticleEmitter::ClassFrameUpdate()
    {
        if (m_bLoop && static_cast<int>((static_cast<double>(m_fLoopInterval) + m_fLoopActivate) * 1024.0) < g_pSysInterface->FrameTime.secs
            && m_iLoopCount > 0)
        {
            --m_iLoopCount;
            const ZMSGID activate = g_pSysInterface->m_pEngineData->RegisterZMsg("Activate", 0, __FILE__, __LINE__);
            SendCommand(activate, nullptr, nullptr);
        }

        const int now = g_pSysInterface->FrameTime.secs;
        if (m_fActivationTime.secs > now)
            return;
        if ((m_fDeactivate != 0.0f && m_fActivationTime.secs - static_cast<int>(m_fDeactivate * -1024.0f) < now)
            || (m_bLoop && m_iLoopCount == 0))
        {
            DisableClassCall(0x10);
            m_bActive = false;
            if (!m_bDeactivate)
                ZGEOM::Delete();
            return;
        }

        UpdateAnimatedParameters();
        const int frameTime = g_pSysInterface->FrameTime.secs;
        const int lifetimeTicks = static_cast<int>(m_fTotalLifeTime * 1024.0f);
        if (frameTime - m_fEmitterTime.secs > lifetimeTicks)
            m_fEmitterTime.secs = frameTime - lifetimeTicks;
        if (m_fEmitterTime.secs - frameTime > static_cast<int>(m_fInterval * 1024.0f))
            m_fEmitterTime.secs = frameTime - static_cast<int>(m_fInterval * -1024.0f);

        float elapsed = static_cast<float>(frameTime - m_fEmitterTime.secs) * TIMETYPE::kInvTPS;
        int emitCount;
        if (m_bEmitted)
        {
            // PC keeps the quotient in x87 until truncation; rounding it to float
            // first can add an emission (for example, 1 second / 0.1f).
            emitCount = static_cast<int>(static_cast<double>(elapsed) / m_fInterval);
            if (emitCount <= 0)
                return;
            ZASSERT(elapsed > EmitterEpsilon);
        }
        else
        {
            m_bEmitted = true;
            emitCount = 1;
            if (elapsed < EmitterEpsilon)
                elapsed = 0.016666668f;
            else
                ZASSERT(elapsed > EmitterEpsilon);
        }

        ZMat3x3 emitterMatrix;
        ZVector3 emitterPosition;
        if (BaseGeom()->Control() & ZCOWNERDRAW)
        {
            ZGROUP* parent = BaseGeom()->ParentGroup();
            if (parent->IsDerivedFrom<ZItem>())
                static_cast<ZItem*>(parent)->GetMainItemRootTM(emitterMatrix, emitterPosition);
            else
                GetRootTM(emitterMatrix, emitterPosition);
        }
        else
            GetRootTM(emitterMatrix, emitterPosition);

        if (vzero(m_vOldParticlePos))
            m_vOldParticlePos = emitterPosition;
        ZVector3 inheritedSpeed;
        vsub(inheritedSpeed, emitterPosition, m_vOldParticlePos);
        vscalar(inheritedSpeed, m_fSpeedInherit / elapsed);

        if (vlen2(m_vExplosionSplash) != 0.0f)
        {
            ZVector3 direction;
            if (std::fabs(m_vRandomPosition.x) < EmitterEpsilon)
                vcpy(direction, &emitterMatrix.data[6]);
            else if (std::fabs(m_vRandomPosition.y) < EmitterEpsilon)
                vcpy(direction, &emitterMatrix.data[3]);
            else if (std::fabs(m_vRandomPosition.z) < EmitterEpsilon)
                vcpy(direction, &emitterMatrix.data[0]);
            if (vdot(direction, m_vExplosionSplash) > 0.0f)
                vscalar(direction, -1.0f);
            vscalar(direction, vlen(m_vExplosionSplash));
            vadd(inheritedSpeed, direction);
        }

        auto* controller = static_cast<ZParticleController*>(RefToPtr(m_rParticleController));
        float fraction = 0.0f;
        const float fractionStep = 1.0f / static_cast<float>(emitCount);
        for (int emission = 0; emission < emitCount; ++emission, fraction += fractionStep)
        {
            ZVector3 position;
            vsub(position, emitterPosition, m_vOldParticlePos);
            vscalar(position, fraction);
            vadd(position, m_vOldParticlePos);
            const int timeOffset = static_cast<int>(static_cast<double>(fraction) * elapsed * -1024.0);

            for (int particle = 0; particle < m_lNumParticlesPerEmit; ++particle)
            {
                ZMat3x3 matrix = emitterMatrix;
                ZVector3 rotation;
                // Keep the PC RNG order, including draws for zero-sized spreads.
                rotation.x = (2.0f * g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 1.0f) * m_vRandomRotation.x;
                rotation.y = (2.0f * g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 1.0f) * m_vRandomRotation.y;
                rotation.z = (2.0f * g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) - 1.0f) * m_vRandomRotation.z;
                // PC mrot (0x4371C0) rotates the X, Y and Z axes in this order.
                vrot(&matrix.data[6], rotation);
                vrot(&matrix.data[3], rotation);
                vrot(&matrix.data[0], rotation);

                ZVector3 particlePosition;
                particlePosition.x = g_pSysInterface->FRand1(const_cast<char*>(__FILE__), __LINE__);
                particlePosition.y = g_pSysInterface->FRand1(const_cast<char*>(__FILE__), __LINE__);
                particlePosition.z = g_pSysInterface->FRand1(const_cast<char*>(__FILE__), __LINE__);
                if (!m_bSquareSpread && vnorm(particlePosition) < EmitterEpsilon)
                    vset(particlePosition, 1.0f, 0.0f, 0.0f);
                particlePosition.x *= m_vRandomPosition.x;
                particlePosition.y *= m_vRandomPosition.y;
                particlePosition.z *= m_vRandomPosition.z;
                vmmul(particlePosition, matrix);
                vadd(particlePosition, position);

                const float speedLength = vlen(m_vSpeed);
                const float speedVariation = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * m_fSpeedVariation;
                ZVector3 velocity(0.0f, 0.0f, (1.0f - speedVariation) * speedLength);
                ZVector3 speedDirection;
                vmmul(speedDirection, m_vSpeed, matrix);
                ZMat3x3 speedMatrix;
                createmat(speedMatrix, speedDirection, nullptr);
                const float innerAngle = m_fInnerConeAngle;
                const float outerAngle = m_fOuterConeAngle;
                const float azimuth = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * 6.2831855f;
                const float coneAngle = g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * (outerAngle - innerAngle) + innerAngle;
                const float sinCone = std::sin(coneAngle);
                ZVector3 coneDirection(sinCone * std::cos(azimuth), sinCone * std::sin(azimuth), std::cos(coneAngle));
                ZMat3x3 coneMatrix;
                createmat(coneMatrix, coneDirection, nullptr);
                vmmul(velocity, coneMatrix);
                vmmul(velocity, speedMatrix);
                vadd(velocity, inheritedSpeed);

                TIMETYPE particleTime;
                particleTime.secs = m_fEmitterTime.secs - timeOffset;
                if (particleTime.secs > g_pSysInterface->FrameTime.secs)
                    particleTime = g_pSysInterface->FrameTime;
                if (controller)
                    controller->CreateParticle(m_lParticleControllerIndex, particlePosition, velocity, particleTime);
            }
        }

        m_fEmitterTime.secs += static_cast<int>(static_cast<double>(static_cast<float>(emitCount)) * m_fInterval * 1024.0);
        vreset(m_vExplosionSplash);
        m_vOldParticlePos = emitterPosition;
    }

    namespace ParticleEmitterProperties
    {
        // PC property order and filters are significant for positional streams.
        // Source-style names aid inspection; PC retail stores null name pointers.
        using Float3 = float[3];
#       define EMITTER_PROPERTY(Name, Next, Type, Table, Filter) \
            static RTP::ZDataProperty<Type> Name { \
                .m_Node = { .m_Next = Next, .m_Name = #Name, .m_Filter = Filter }, \
                .m_VirtualTable = &RTP::VirtualTables::Table, \
                .m_Offset = reinterpret_cast<Type*>(CLASS_PROPERTY(ZParticleEmitter, Name)) }
#       define EMITTER_FLAG(Name, Next, PropertyName) \
            static RTP::ZVirtualProperty<bool> Name { \
                .m_Node = { .m_Next = Next, .m_Name = PropertyName, .m_Filter = 1 }, \
                .m_VirtualTable = &RTP::VirtualTables::Virtual_bool, \
                .m_Get = &ZParticleEmitter::Get##Name, .m_Set = &ZParticleEmitter::Set##Name }

        EMITTER_PROPERTY(m_fTotalLifeTime, nullptr, float, Data_float, 2);
        EMITTER_PROPERTY(m_vOldParticlePos, m_fTotalLifeTime, Float3, Data_float_3, 2);
        EMITTER_PROPERTY(m_lParticleControllerIndex, m_vOldParticlePos, uint, Data_uint, 2);
        EMITTER_PROPERTY(m_rParticleController, m_lParticleControllerIndex, ZGEOMREF, Data_ZGEOMREF, 2);
        EMITTER_PROPERTY(m_vExplosionSplash, m_rParticleController, Float3, Data_float_3, 2);
        EMITTER_PROPERTY(m_fEmitterTime, m_vExplosionSplash, TIMETYPE, Data_TIMETYPE, 2);
        EMITTER_PROPERTY(m_fActivationTime, m_fEmitterTime, TIMETYPE, Data_TIMETYPE, 2);
        static RTP::ZVirtualProperty<ZRTString> szParticleTemplateName {
            .m_Node = { .m_Next = m_fActivationTime, .m_Name = "szParticleTemplateName", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Virtual_ZRTString,
            .m_Get = &ZParticleEmitter::GetszParticleTemplateName,
            .m_Set = &ZParticleEmitter::SetszParticleTemplateName };
        EMITTER_FLAG(bDeactivate, szParticleTemplateName, "bDeactivate");
        EMITTER_FLAG(bAutoStart, bDeactivate, "bAutoStart");
        EMITTER_FLAG(bLighting, bAutoStart, "bLighting");
        EMITTER_FLAG(bHide, bLighting, "bHide");
        EMITTER_FLAG(bGlobal, bHide, "IsGlobal");
        EMITTER_PROPERTY(m_msgActivate, bGlobal, ZMsg, Data_ZMsg, 1);
        EMITTER_PROPERTY(m_bSquareSpread, m_msgActivate, bool, Data_bool, 1);
        EMITTER_PROPERTY(m_bExpandBounds, m_bSquareSpread, bool, Data_bool, 1);
        EMITTER_PROPERTY(m_iLoopCount, m_bExpandBounds, int, Data_int, 1);
        EMITTER_PROPERTY(m_fLoopInterval, m_iLoopCount, float, Data_float, 1);
        EMITTER_PROPERTY(m_bLoop, m_fLoopInterval, bool, Data_bool, 1);
        EMITTER_PROPERTY(m_iAnimOffsetSpeed, m_bLoop, uint, Data_uint, 1);
        EMITTER_PROPERTY(m_iAnimOffsetEPS, m_iAnimOffsetSpeed, uint, Data_uint, 1);
        EMITTER_PROPERTY(m_rParticleTemplate, m_iAnimOffsetEPS, ZGEOMREF, Data_ZGEOMREF, 1);
        EMITTER_PROPERTY(m_fStartDelay, m_rParticleTemplate, float, Data_float, 1);
        EMITTER_PROPERTY(m_fSpeedInherit, m_fStartDelay, float, Data_float, 1);
        EMITTER_PROPERTY(m_fSpeedVariation, m_fSpeedInherit, float, Data_float, 1);
        EMITTER_PROPERTY(m_vSpeed, m_fSpeedVariation, Float3, Data_float_3, 1);
        EMITTER_PROPERTY(m_fOuterConeAngle, m_vSpeed, float, Data_float, 1);
        EMITTER_PROPERTY(m_fInnerConeAngle, m_fOuterConeAngle, float, Data_float, 1);
        EMITTER_PROPERTY(m_fDeactivate, m_fInnerConeAngle, float, Data_float, 1);
        EMITTER_PROPERTY(m_fInterval, m_fDeactivate, float, Data_float, 1);
        EMITTER_PROPERTY(m_vRandomRotation, m_fInterval, Float3, Data_float_3, 1);
        EMITTER_PROPERTY(m_lNumParticlesPerEmit, m_vRandomRotation, short, Data_short, 1);
        EMITTER_PROPERTY(m_vRandomPosition, m_lNumParticlesPerEmit, Float3, Data_float_3, 1);
#       undef EMITTER_FLAG
#       undef EMITTER_PROPERTY
    }

    STATIC_CLASS_VAR_IMPL(ZParticleEmitter, ZMessageResolver, s_msgDeactivate, 0x009730C0, { "Deactivate" });
    STATIC_CLASS_VAR_IMPL(ZParticleEmitter, ZMessageResolver, s_msgExplosionSplash, 0x0097311C, { "ExplosionSplash" });
    DECLARE_GEOM_CLASS_IMPL(ZParticleEmitter, ZSTDOBJ, 0x00973118, "ZParticleEmitter", 0x0076CC88,
        ParticleEmitterProperties::m_vRandomPosition, 0x00806EBC, 0x009730B8, 0x009730BC);
}
