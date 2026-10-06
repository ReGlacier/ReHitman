#include <Glacier/Geom/ZParticleEmitter.h>
#include <Glacier/Geom/ZParticleController.h>
#include <Glacier/Geom/ZParticleTemplate.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/ZUniMemory.h>
#include <Tests/EngineFixture.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZParticleEmitter) == 0xA4);

    struct TestEmitter : ZParticleEmitter
    {
        explicit TestEmitter(ZBaseGeom* base) : ZParticleEmitter("emitter-test", base)
        {
            // RTP normally supplies these fields; ZUniMemory::New does not zero storage.
            m_fLoopActivate = 0.0f;
            m_vRandomPosition.Reset();
            m_lNumParticlesPerEmit = 1;
            m_vRandomRotation.Reset();
            m_fInterval = 0.25f;
            m_fDeactivate = 0.0f;
            m_fInnerConeAngle = m_fOuterConeAngle = 0.0f;
            m_vSpeed = ZVector3(0.0f, 0.0f, 1.0f);
            m_fSpeedVariation = m_fSpeedInherit = m_fStartDelay = 0.0f;
            m_iAnimOffsetEPS = m_iAnimOffsetSpeed = 0xFFFFFFFFu;
            m_bLoop = false;
            m_fLoopInterval = 0.0f;
            m_iLoopCount = 0;
            m_bExpandBounds = m_bSquareSpread = false;
            m_msgActivate = 0;
            m_bGlobal = m_bHide = m_bLighting = m_bAutoStart = false;
            m_bDeactivate = true;
            m_rParticleController = 0;
            m_lParticleControllerIndex = 0;
            m_fTotalLifeTime = 10.0f;
        }

        using ZParticleEmitter::UpdateAnimatedParameterFloat;
        using ZParticleEmitter::UpdateAnimatedParameterVector;
        using ZParticleEmitter::UpdateAnimatedParameters;
    };

    struct CapturingController : ZParticleController
    {
        explicit CapturingController(ZBaseGeom* base) : ZParticleController("controller-test", base) {}

        struct Emission
        {
            uint32_t Index;
            ZVector3 Position;
            ZVector3 Velocity;
            TIMETYPE Time;
        };

        void CreateParticle(uint32_t index, const float* position, const float* velocity, TIMETYPE time) override
        {
            ASSERT_LT(Count, 16u);
            Emissions[Count++] = { index, ZVector3(position), ZVector3(velocity), time };
        }

        Emission Emissions[16]{};
        unsigned Count = 0;
    };

    void ExpectVector(const float* actual, float x, float y, float z)
    {
        EXPECT_FLOAT_EQ(actual[0], x);
        EXPECT_FLOAT_EQ(actual[1], y);
        EXPECT_FLOAT_EQ(actual[2], z);
    }

    class ZParticleEmitterTest : public Tests::EngineFixture
    {
    protected:
        void SetUp() override
        {
            EngineFixture::SetUp();
            PreviousDeactivate = ZParticleEmitter::s_msgDeactivate.m_MessageID;
            PreviousSplash = ZParticleEmitter::s_msgExplosionSplash.m_MessageID;
            ZParticleEmitter::s_msgDeactivate.m_MessageID = EngineData().RegisterZMsg("Deactivate", 0, __FILE__, __LINE__);
            ZParticleEmitter::s_msgExplosionSplash.m_MessageID = EngineData().RegisterZMsg("ExplosionSplash", 0, __FILE__, __LINE__);
            EngineData().m_pGeomBuffer = ZUniMemory::New<ZGeomBuffer>(sizeof(ZBaseGeom) * 4, 4096, 512, 4096);
            Emitter = ZUniMemory::New<TestEmitter>(MakeBase());
            Emitter->m_msgActivate = EngineData().RegisterZMsg("Activate", 0, __FILE__, __LINE__);
            SysInterface().FrameTime.secs = 0;
            SysInterface().SRand(1, __FILE__, __LINE__);
        }

        void TearDown() override
        {
            DestroyGeom(Source);
            DestroyGeom(Controller);
            DestroyGeom(Emitter);
            ZParticleEmitter::s_msgDeactivate.m_MessageID = PreviousDeactivate;
            ZParticleEmitter::s_msgExplosionSplash.m_MessageID = PreviousSplash;
            EngineFixture::TearDown();
        }

        ZBaseGeom* MakeBase()
        {
            // The real geom pool supplies raw slots, requiring placement construction.
            return znew_placement(EngineData().m_pGeomBuffer->AllocBaseGeom());
        }

        template <typename T>
        void DestroyGeom(T*& geom)
        {
            if (!geom)
                return;
            auto* base = geom->BaseGeom();
            // Extra geoms here belong to ZUniMemory, not the extra-geom pool. Keep
            // the mutually owning destructors from invoking pooled extra deletion.
            base->m_lControl |= 0x80000000u;
            ZUniMemory::Delete(geom);
            base->m_pExtraGeom = nullptr;
            base->~ZBaseGeom();
            geom = nullptr;
        }

        void CaptureParticles()
        {
            Controller = ZUniMemory::New<CapturingController>(MakeBase());
            Emitter->m_rParticleController = EngineData().m_pGeomBuffer->GeomPtrToRef(Controller);
            Emitter->m_lParticleControllerIndex = 7;
            Emitter->BaseGeom()->m_vPos = ZVector3(2.0f, 3.0f, 4.0f);
        }

        void InstallAnimation()
        {
            // Two scalar samples (-1, +1), followed by three constant float
            // channels (+1, -1, +1). Both use the real StreamPacker format.
            auto* data = static_cast<uint8_t*>(ZUniMemory::Allocate(96));
            std::memset(data, 0, 96);
            EngineData().m_pStaticBuffer = data;
            EngineData().m_lStaticBufferLength = 96;
            const ZParticleEmitterAnimationHeader header{10, 12};
            std::memcpy(data, &header, sizeof(header));
            std::memcpy(data + 48, &header, sizeof(header));

            auto* scalar = data + sizeof(header);
            scalar[0] = 1;
            scalar[3] = 0x11; // One-bit precision, float layout.
            scalar[6] = 1;   // One metadata word before the block table.
            scalar[8] = 2;   // Midstream begins two bytes after the first stream.
            scalar[18] = 0x12; // Two-bit deltas; first delta is +1.

            auto* vector = data + 48 + sizeof(header);
            vector[0] = 3;
            vector[3] = 1;
            vector[5] = 2;
            vector[7] = vector[8] = vector[9] = 0x11;
            vector[12] = 1; // One metadata word before the block table.
            vector[16] = 0x37; // (value << 1) | constant for values 1, 0, 1.
        }

        TestEmitter* Emitter = nullptr;
        TestEmitter* Source = nullptr;
        CapturingController* Controller = nullptr;
        uint32_t PreviousDeactivate = 0;
        uint32_t PreviousSplash = 0;
    };
}

namespace
{
    struct SchedulingEmitter : TestEmitter
    {
        explicit SchedulingEmitter(ZBaseGeom* base) : TestEmitter(base) {}

        void EnableClassCall(uint32_t cases) override
        {
            EXPECT_EQ(cases, 0x10u);
            ++EnableRequests;
        }

        void DisableClassCall(uint32_t cases) override
        {
            EXPECT_EQ(cases, 0x10u);
            ++DisableRequests;
        }

        using TestEmitter::SendCommand;
        void SendCommand(ZMSGID msg, void* data, ZGEOM* target) override
        {
            EXPECT_EQ(target, nullptr);
            EXPECT_EQ(msg, m_msgActivate);
            ++RestartRequests;
            ClassCommand(msg, data);
        }

        unsigned EnableRequests = 0;
        unsigned DisableRequests = 0;
        unsigned RestartRequests = 0;
    };

    struct TemplateGameData : ZGameData
    {
        TEnumID GetAmmoEnumId(const char*) override { return TEnumID{}; }
    };

    class ZParticleEmitterInitTest : public ZParticleEmitterTest
    {
    protected:
        void SetUp() override
        {
            ZParticleEmitterTest::SetUp();
            const ZMSGID activate = Emitter->m_msgActivate;
            DestroyGeom(Emitter);
            Scheduled = ZUniMemory::New<SchedulingEmitter>(MakeBase());
            Emitter = Scheduled;
            Emitter->m_msgActivate = activate;
            PreviousGameData = g_pGameData;
        }

        void TearDown() override
        {
            // Only the transform parent is needed here, not group-list ownership.
            if (Parent)
                Emitter->BaseGeom()->m_pParent = nullptr;
            DestroyGeom(Parent);
            DestroyGeom(FirstTemplate);
            DestroyGeom(NextTemplate);
            g_pGameData = PreviousGameData;
            ZUniMemory::Delete(GameData);
            ZParticleEmitterTest::TearDown();
        }

        void BindTemplate()
        {
            CaptureParticles();
            EngineData().m_rParticleControllerGeom = Emitter->m_rParticleController;
            FirstTemplate = ZUniMemory::New<ZParticleTemplate>("first-template", MakeBase());
            FirstTemplate->m_fMaxAge = 1.25f;
            FirstTemplate->m_rNextTemplate = 0;
            Emitter->m_rParticleTemplate = FirstTemplate->GetRef();
        }

        SchedulingEmitter* Scheduled = nullptr;
        ZParticleTemplate* FirstTemplate = nullptr;
        ZParticleTemplate* NextTemplate = nullptr;
        ZGROUP* Parent = nullptr;
        TemplateGameData* GameData = nullptr;
        ZGameData* PreviousGameData = nullptr;
    };

    class StableActivateEngineData : public ZEngineDataBase
    {
    public:
        StableActivateEngineData(ZEngineDataBase& original, ZMSGID activate)
            : ZEngineDataBase(""), Original(original), Activate(activate)
        {
        }

        ZMSGID RegisterZMsg(const char* name, uint32_t forcedValue, const char* file, int line) override
        {
            // The production registry retains a pointer into its temporary MYSTR.
            // Isolate that dangling-key bug, not the emitter's message dispatch.
            if (name && _stricmp(name, "Activate") == 0)
                return Activate;
            return Original.RegisterZMsg(name, forcedValue, file, line);
        }

    private:
        ZEngineDataBase& Original;
        const ZMSGID Activate;
    };

    class ZParticleEmitterLoopTest : public ZParticleEmitterInitTest
    {
    protected:
        void SetUp() override
        {
            ZParticleEmitterInitTest::SetUp();
            LoopEngineData = ZUniMemory::New<StableActivateEngineData>(EngineData(), Emitter->m_msgActivate);
            SysInterface().m_pEngineData = LoopEngineData;
            // The base engine-data constructor reseeds the system RNG.
            SysInterface().SRand(1, __FILE__, __LINE__);
        }

        void TearDown() override
        {
            SysInterface().m_pEngineData = &EngineData();
            ZUniMemory::Delete(LoopEngineData);
            ZParticleEmitterInitTest::TearDown();
        }

        // No engine pointers are copied: EngineFixture retains the real geom
        // buffer, and ZGEOM::RefToPtr continues to resolve through its singleton.
        StableActivateEngineData* LoopEngineData = nullptr;
    };
}

TEST_F(ZParticleEmitterTest, PcFlagBitsOccupyByte74WithoutTouchingClocks)
{
    auto* bytes = reinterpret_cast<unsigned char*>(Emitter);
    EXPECT_EQ(reinterpret_cast<unsigned char*>(&Emitter->m_msgActivate) - bytes, 0x72);
    EXPECT_EQ(reinterpret_cast<unsigned char*>(&Emitter->m_fActivationTime) - bytes, 0x78);
    EXPECT_EQ(reinterpret_cast<unsigned char*>(&Emitter->m_fEmitterTime) - bytes, 0x7C);
    EXPECT_EQ(reinterpret_cast<unsigned char*>(&Emitter->m_rParticleController) - bytes, 0x8C);
    Emitter->m_fActivationTime.secs = 123;
    Emitter->m_fEmitterTime.secs = 456;
    Emitter->m_bDeactivate = false;
    EXPECT_FALSE(Emitter->GetActive());
    EXPECT_FALSE(Emitter->m_bEmitted);
    EXPECT_EQ(bytes[0x74] & 0x7F, 0);

    Emitter->SetbGlobal(true);
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x01);
    Emitter->SetbHide(true);
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x03);
    Emitter->SetbLighting(true);
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x07);
    Emitter->SetbAutoStart(true);
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x0F);
    Emitter->SetbDeactivate(true);
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x1F);
    Emitter->m_bActive = true;
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x3F);
    Emitter->m_bEmitted = true;
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x7F);
    Emitter->SetbHide(false);
    EXPECT_EQ(bytes[0x74] & 0x7F, 0x7D);
    EXPECT_FALSE(Emitter->BaseGeom()->IsHidden());
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 123);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 456);
    bool value = false;
    Emitter->GetbGlobal(value);
    EXPECT_TRUE(value);
    Emitter->GetbHide(value);
    EXPECT_FALSE(value);
    Emitter->GetbLighting(value);
    EXPECT_TRUE(value);
    Emitter->GetbAutoStart(value);
    EXPECT_TRUE(value);
    Emitter->GetbDeactivate(value);
    EXPECT_TRUE(value);
}

TEST_F(ZParticleEmitterTest, ClassInitConvertsDegreesAndSeparatesEqualIntervals)
{
    Emitter->m_fInterval = Emitter->m_fDeactivate = 2.0f;
    Emitter->m_fInnerConeAngle = 90.0f;
    Emitter->m_fOuterConeAngle = 180.0f;
    Emitter->m_vRandomRotation = ZVector3(-90.0f, 0.0f, 360.0f);
    Emitter->ClassInit();
    EXPECT_TRUE(Emitter->BaseGeom()->WantCameraMsg());
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, 2.1f);
    EXPECT_FLOAT_EQ(Emitter->m_fDeactivate, 2.0f);
    EXPECT_NEAR(Emitter->m_fInnerConeAngle, 1.5707963f, 1e-6f);
    EXPECT_NEAR(Emitter->m_fOuterConeAngle, 3.1415927f, 1e-6f);
    EXPECT_NEAR(Emitter->m_vRandomRotation.x, -1.5707963f, 1e-6f);
    EXPECT_FLOAT_EQ(Emitter->m_vRandomRotation.y, 0.0f);
    EXPECT_NEAR(Emitter->m_vRandomRotation.z, 6.2831853f, 1e-6f);
}

TEST_F(ZParticleEmitterTest, ClassInitLeavesDistinctIntervalUnchanged)
{
    Emitter->m_fInterval = 0.5f;
    Emitter->m_fDeactivate = 0.75f;
    Emitter->ClassInit();
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, 0.5f);
}

TEST_F(ZParticleEmitterTest, LocalActivationAndReactivationResetRuntimeState)
{
    Emitter->ClassInit();
    SysInterface().FrameTime.secs = 2048;
    Emitter->m_fStartDelay = 0.125f;
    Emitter->m_bEmitted = true;
    Emitter->m_vOldParticlePos = ZVector3(1.0f, 2.0f, 3.0f);
    Emitter->m_vExplosionSplash = ZVector3(4.0f, 5.0f, 6.0f);
    EXPECT_EQ(Emitter->ClassCommand(Emitter->m_msgActivate, nullptr), 0);
    EXPECT_TRUE(Emitter->GetActive());
    EXPECT_FALSE(Emitter->m_bEmitted);
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 2176);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 2176);
    EXPECT_FLOAT_EQ(Emitter->m_fLoopActivate, 2.0f);
    ExpectVector(Emitter->m_vOldParticlePos, 0.0f, 0.0f, 0.0f);
    ExpectVector(Emitter->m_vExplosionSplash, 0.0f, 0.0f, 0.0f);
    EXPECT_EQ(Emitter->m_pExData->_Events.Count(), 0u);

    SysInterface().FrameTime.secs = 4096;
    Emitter->m_bEmitted = true;
    Emitter->ClassCommand(Emitter->m_msgActivate, nullptr);
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 4224);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 4224);
    EXPECT_FALSE(Emitter->m_bEmitted);
}

// Scheduling a real class-call is deliberately excluded: ZGEOM's event walkers
// stop on 1, while ZGeomEventList returns 0 at end-of-run. Exercise command state
// without creating an event that DisableClassCall cannot safely traverse.
TEST_F(ZParticleEmitterTest, DeactivateAndSafeViewCommandsPreserveActivationClock)
{
    Emitter->ClassInit();
    Emitter->ClassCommand(Emitter->m_msgActivate, nullptr);
    Emitter->m_vOldParticlePos = ZVector3(1.0f, 2.0f, 3.0f);
    Emitter->ClassCommand(2053, nullptr);
    ExpectVector(Emitter->m_vOldParticlePos, 0.0f, 0.0f, 0.0f);
    EXPECT_TRUE(Emitter->GetActive());
    Emitter->ClassCommand(static_cast<ZMSGID>(ZParticleEmitter::s_msgDeactivate), nullptr);
    EXPECT_FALSE(Emitter->GetActive());
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 0);
    Emitter->m_vOldParticlePos = ZVector3(4.0f, 5.0f, 6.0f);
    Emitter->ClassCommand(2054, nullptr); // An inactive local emitter must not schedule updates.
    Emitter->ClassCommand(2053, nullptr);
    EXPECT_EQ(Emitter->m_pExData->_Events.Count(), 0u);
    ExpectVector(Emitter->m_vOldParticlePos, 4.0f, 5.0f, 6.0f);

    Emitter->m_bGlobal = Emitter->m_bActive = true;
    Emitter->ClassCommand(2053, nullptr);
    Emitter->ClassCommand(2054, nullptr);
    ExpectVector(Emitter->m_vOldParticlePos, 4.0f, 5.0f, 6.0f);
    EXPECT_EQ(Emitter->m_pExData->_Events.Count(), 0u);
}

TEST_F(ZParticleEmitterTest, ExplosionSplashCommandCopiesPayload)
{
    float splash[] = {3.0f, -4.0f, 5.0f};
    EXPECT_EQ(Emitter->ClassCommand(static_cast<ZMSGID>(ZParticleEmitter::s_msgExplosionSplash), splash), 0);
    splash[0] = 99.0f;
    ExpectVector(Emitter->m_vExplosionSplash, 3.0f, -4.0f, 5.0f);
    EXPECT_FALSE(Emitter->GetActive());
}

TEST_F(ZParticleEmitterTest, AnimationSentinelsLeaveOutputsAndParametersUntouched)
{
    float scalar = 42.0f;
    float vector[] = {2.0f, 3.0f, 4.0f};
    EXPECT_FALSE(Emitter->UpdateAnimatedParameterFloat(-1, 100, &scalar));
    EXPECT_FALSE(Emitter->UpdateAnimatedParameterVector(-1, 100, vector));
    EXPECT_FLOAT_EQ(scalar, 42.0f);
    ExpectVector(vector, 2.0f, 3.0f, 4.0f);
    Emitter->UpdateAnimatedParameters();
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, 0.25f);
    ExpectVector(Emitter->Speed(), 0.0f, 0.0f, 1.0f);
}

TEST_F(ZParticleEmitterTest, AnimationRejectsBeforeStartAndClampsAtExclusiveEnd)
{
    InstallAnimation();
    float scalar = 42.0f;
    float vector[] = {2.0f, 3.0f, 4.0f};
    EXPECT_FALSE(Emitter->UpdateAnimatedParameterFloat(0, 9, &scalar));
    EXPECT_FALSE(Emitter->UpdateAnimatedParameterVector(48, 9, vector));
    EXPECT_FLOAT_EQ(scalar, 42.0f);
    ExpectVector(vector, 2.0f, 3.0f, 4.0f);
    ASSERT_TRUE(Emitter->UpdateAnimatedParameterFloat(0, 10, &scalar));
    EXPECT_FLOAT_EQ(scalar, -1.0f);
    for (int frame : {11, 12, 100})
    {
        SCOPED_TRACE(frame);
        ASSERT_TRUE(Emitter->UpdateAnimatedParameterFloat(0, frame, &scalar));
        EXPECT_FLOAT_EQ(scalar, 1.0f);
        ASSERT_TRUE(Emitter->UpdateAnimatedParameterVector(48, frame, vector));
        ExpectVector(vector, 1.0f, -1.0f, 1.0f);
    }
}

TEST_F(ZParticleEmitterTest, AnimatedParametersUseActivationRelative25FpsAndRateReciprocal)
{
    InstallAnimation();
    Emitter->m_iAnimOffsetEPS = 0;
    Emitter->m_iAnimOffsetSpeed = 48;
    Emitter->m_fActivationTime.secs = 1024;
    SysInterface().FrameTime.secs = 1023;
    Emitter->UpdateAnimatedParameters();
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, 0.25f);
    ExpectVector(Emitter->Speed(), 0.0f, 0.0f, 1.0f);
    SysInterface().FrameTime.secs = 1024 + 410; // Truncates to animation frame 10.
    Emitter->UpdateAnimatedParameters();
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, -1.0f);
    ExpectVector(Emitter->Speed(), 1.0f, -1.0f, 1.0f);
    SysInterface().FrameTime.secs = 1024 + 451; // Truncates to frame 11.
    Emitter->UpdateAnimatedParameters();
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, 1.0f);
}

TEST_F(ZParticleEmitterTest, PropertiesRetainPcOrderAndContentFilters)
{
    const char* names[] = {
        "m_vRandomPosition", "m_lNumParticlesPerEmit", "m_vRandomRotation",
        "m_fInterval", "m_fDeactivate", "m_fInnerConeAngle", "m_fOuterConeAngle",
        "m_vSpeed", "m_fSpeedVariation", "m_fSpeedInherit", "m_fStartDelay",
        "m_rParticleTemplate", "m_iAnimOffsetEPS", "m_iAnimOffsetSpeed", "m_bLoop",
        "m_fLoopInterval", "m_iLoopCount", "m_bExpandBounds", "m_bSquareSpread",
        "m_msgActivate", "IsGlobal", "bHide", "bLighting", "bAutoStart", "bDeactivate",
        "szParticleTemplateName", "m_fActivationTime", "m_fEmitterTime",
        "m_vExplosionSplash", "m_rParticleController", "m_lParticleControllerIndex",
        "m_vOldParticlePos", "m_fTotalLifeTime"
    };
    const auto& info = ZParticleEmitter::Properties();
    EXPECT_EQ(&Emitter->GetProperties(), &info);
    EXPECT_EQ(info.Super, &ZSTDOBJ::Info);
    EXPECT_STREQ(info.Name, "ZParticleEmitter");
    const auto* node = info.First;
    for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
    {
        SCOPED_TRACE(i);
        ASSERT_NE(node, nullptr);
        EXPECT_STREQ(node->m_Name, names[i]);
        EXPECT_EQ(node->m_Filter, i < 26 ? 1u : 2u);
        node = node->m_Next;
    }
    EXPECT_EQ(node, nullptr);
}

TEST_F(ZParticleEmitterTest, CopyDataCopiesConfigurationButOmitsLoopAndRuntimeState)
{
    Source = ZUniMemory::New<TestEmitter>(MakeBase());
    Source->m_bLighting = true;
    Source->m_fInterval = 0.75f;
    Source->m_vSpeed = ZVector3(4.0f, 5.0f, 6.0f);
    Source->m_fActivationTime.secs = 1234;
    Source->m_fEmitterTime.secs = 5678;
    Source->m_vOldParticlePos = ZVector3(7.0f, 8.0f, 9.0f);
    Source->m_iAnimOffsetEPS = 24;
    Source->m_iAnimOffsetSpeed = 48;
    Source->m_bLoop = true;
    Source->m_fLoopInterval = 9.0f;
    Source->m_iLoopCount = 8;
    Source->m_bSquareSpread = true;
    Source->m_fLoopActivate = 7.0f;
    Emitter->m_fLoopInterval = 2.0f;
    Emitter->m_iLoopCount = 3;
    Emitter->m_fLoopActivate = 4.0f;
    Emitter->m_bActive = Emitter->m_bEmitted = true;
    Emitter->m_fEmitterTime.secs = 99;
    Emitter->m_vExplosionSplash = ZVector3(1.0f, 2.0f, 3.0f);

    Emitter->CopyData(Source);

    EXPECT_TRUE(Emitter->m_bLighting);
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, 0.75f);
    ExpectVector(Emitter->Speed(), 4.0f, 5.0f, 6.0f);
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 1234);
    EXPECT_EQ(Emitter->m_iAnimOffsetEPS, 24u);
    EXPECT_EQ(Emitter->m_iAnimOffsetSpeed, 48u);
    EXPECT_FALSE(Emitter->m_bLoop);
    EXPECT_FALSE(Emitter->m_bSquareSpread);
    EXPECT_FLOAT_EQ(Emitter->m_fLoopInterval, 2.0f);
    EXPECT_EQ(Emitter->m_iLoopCount, 3);
    EXPECT_FLOAT_EQ(Emitter->m_fLoopActivate, 4.0f);
    EXPECT_TRUE(Emitter->m_bActive);
    EXPECT_TRUE(Emitter->m_bEmitted);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 99);
    ExpectVector(Emitter->m_vExplosionSplash, 1.0f, 2.0f, 3.0f);
    // FinishInit resets the old position even though CopyData initially copies it.
    ExpectVector(Emitter->m_vOldParticlePos, 0.0f, 0.0f, 0.0f);
}

TEST_F(ZParticleEmitterTest, FrameUpdateWaitsForActivationThenEmitsExactlyOnce)
{
    CaptureParticles();
    Emitter->m_fActivationTime.secs = Emitter->m_fEmitterTime.secs = 1024;
    SysInterface().FrameTime.secs = 1023;
    Emitter->ClassFrameUpdate();
    EXPECT_EQ(Controller->Count, 0u);
    EXPECT_FALSE(Emitter->m_bEmitted);
    SysInterface().FrameTime.secs = 1024;
    Emitter->ClassFrameUpdate();
    ASSERT_EQ(Controller->Count, 1u);
    EXPECT_TRUE(Emitter->m_bEmitted);
    EXPECT_EQ(Controller->Emissions[0].Index, 7u);
    EXPECT_EQ(Controller->Emissions[0].Time.secs, 1024);
    ExpectVector(Controller->Emissions[0].Position, 2.0f, 3.0f, 4.0f);
    ExpectVector(Controller->Emissions[0].Velocity, 0.0f, 0.0f, 1.0f);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 1280);
    Emitter->ClassFrameUpdate();
    EXPECT_EQ(Controller->Count, 1u);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 1280);
}

TEST_F(ZParticleEmitterTest, FrameUpdateCatchesUpUsingLifetimeClampAndParticleCount)
{
    CaptureParticles();
    Emitter->m_bEmitted = true;
    Emitter->m_lNumParticlesPerEmit = 2;
    Emitter->m_fTotalLifeTime = 0.5f;
    Emitter->m_fEmitterTime.secs = 0;
    SysInterface().FrameTime.secs = 2048;
    Emitter->ClassFrameUpdate();
    ASSERT_EQ(Controller->Count, 4u);
    EXPECT_EQ(Controller->Emissions[0].Time.secs, 1536);
    EXPECT_EQ(Controller->Emissions[1].Time.secs, 1536);
    EXPECT_EQ(Controller->Emissions[2].Time.secs, 1792);
    EXPECT_EQ(Controller->Emissions[3].Time.secs, 1792);
    for (unsigned i = 0; i < Controller->Count; ++i)
    {
        SCOPED_TRACE(i);
        EXPECT_EQ(Controller->Emissions[i].Index, 7u);
        ExpectVector(Controller->Emissions[i].Position, 2.0f, 3.0f, 4.0f);
        ExpectVector(Controller->Emissions[i].Velocity, 0.0f, 0.0f, 1.0f);
    }
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 2048);
    ExpectVector(Emitter->m_vOldParticlePos, 2.0f, 3.0f, 4.0f);
    Emitter->ClassFrameUpdate();
    EXPECT_EQ(Controller->Count, 4u);
}

TEST_F(ZParticleEmitterTest, EmissionCountPreservesPrecisionBeforeIntegerTruncation)
{
    CaptureParticles();
    Emitter->m_bEmitted = true;
    Emitter->m_fInterval = 0.1f;
    Emitter->m_fEmitterTime.secs = 0;
    SysInterface().FrameTime.secs = 1024;

    Emitter->ClassFrameUpdate();

    // The stored float interval is slightly above 0.1: double division gives
    // just under ten, whereas rounding the quotient to float would give ten.
    ASSERT_EQ(Controller->Count, 9u);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 921);
}

TEST_F(ZParticleEmitterTest, EmitterCursorPreservesPrecisionBeforeTickTruncation)
{
    CaptureParticles();
    Emitter->m_bEmitted = true;
    Emitter->m_fInterval = 0.01f;
    Emitter->m_lNumParticlesPerEmit = 0; // Exercise 100 emissions without overflowing capture storage.
    Emitter->m_fEmitterTime.secs = 0;
    SysInterface().FrameTime.secs = 1024;

    Emitter->ClassFrameUpdate();

    // 100 * double(0.01f) * 1024 is just below 1024. A float intermediate
    // rounds the product to one second and incorrectly advances an extra tick.
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 1023);
    EXPECT_EQ(Controller->Count, 0u);
    ExpectVector(Emitter->m_vOldParticlePos, 2.0f, 3.0f, 4.0f);
}

TEST_F(ZParticleEmitterTest, AnimationFrameRetainsLowTicksAtLargeElapsedTimes)
{
    InstallAnimation();
    auto* data = reinterpret_cast<uint8_t*>(EngineData().m_pStaticBuffer);
    const ZParticleEmitterAnimationHeader header{409601, 409603};
    std::memcpy(data, &header, sizeof(header));
    std::memcpy(data + 48, &header, sizeof(header));
    Emitter->m_iAnimOffsetEPS = 0;
    Emitter->m_iAnimOffsetSpeed = 48;
    Emitter->m_fActivationTime.secs = 1024;

    SysInterface().FrameTime.secs = 1024 + 16777256;
    Emitter->UpdateAnimatedParameters();
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, 0.25f);
    ExpectVector(Emitter->Speed(), 0.0f, 0.0f, 1.0f);

    // One tick later is frame 409601. Converting elapsed seconds to float
    // first loses this tick and incorrectly keeps the animation before start.
    SysInterface().FrameTime.secs = 1024 + 16777257;
    Emitter->UpdateAnimatedParameters();
    EXPECT_FLOAT_EQ(Emitter->m_fInterval, -1.0f);
    ExpectVector(Emitter->Speed(), 1.0f, -1.0f, 1.0f);
}

TEST_F(ZParticleEmitterInitTest, ClassInit2WithoutTemplateDoesNotFinishInitialization)
{
    Emitter->m_bAutoStart = Emitter->m_bGlobal = Emitter->m_bHide = true;
    Emitter->m_fTotalLifeTime = 99.0f;
    Emitter->m_lParticleControllerIndex = 17;
    Emitter->m_vOldParticlePos = ZVector3(1.0f, 2.0f, 3.0f);

    Emitter->ClassInit2();

    EXPECT_FALSE(Emitter->GetActive());
    EXPECT_FALSE(Emitter->BaseGeom()->IsHidden());
    EXPECT_FLOAT_EQ(Emitter->m_fTotalLifeTime, 99.0f);
    EXPECT_EQ(Emitter->m_lParticleControllerIndex, 17u);
    ExpectVector(Emitter->m_vOldParticlePos, 1.0f, 2.0f, 3.0f);
    EXPECT_EQ(Scheduled->EnableRequests, 0u);
}

TEST_F(ZParticleEmitterInitTest, ClassInit2RegistersHeadTemplateAndSumsChainLifetime)
{
    BindTemplate();
    NextTemplate = ZUniMemory::New<ZParticleTemplate>("next-template", MakeBase());
    NextTemplate->m_fMaxAge = 2.5f;
    NextTemplate->m_rNextTemplate = 0;
    FirstTemplate->m_rNextTemplate = NextTemplate->GetRef();
    Emitter->m_rParticleController = 0;
    Emitter->m_lParticleControllerIndex = 99;
    Emitter->m_vOldParticlePos = ZVector3(8.0f, 9.0f, 10.0f);

    Emitter->ClassInit2();

    EXPECT_EQ(Emitter->m_rParticleController, Controller->GetRef());
    EXPECT_EQ(Emitter->m_lParticleControllerIndex, 1u);
    EXPECT_EQ(FirstTemplate->m_lControllerIndex, 1u);
    EXPECT_EQ(NextTemplate->m_lControllerIndex, 0u);
    EXPECT_EQ(Controller->m_ParticleManager[0].pParticleTemplate, FirstTemplate->GetRef());
    EXPECT_EQ(Controller->m_ParticleManager[1].pParticleTemplate, 0u);
    EXPECT_FLOAT_EQ(Emitter->m_fTotalLifeTime, 3.75f);
    ExpectVector(Emitter->m_vOldParticlePos, 0.0f, 0.0f, 0.0f);
    EXPECT_FALSE(Emitter->GetActive());

    // A cached template index is reused, and the lifetime is recomputed, not accumulated.
    Emitter->ClassInit2();
    EXPECT_EQ(Emitter->m_lParticleControllerIndex, 1u);
    EXPECT_FLOAT_EQ(Emitter->m_fTotalLifeTime, 3.75f);
    EXPECT_EQ(Controller->m_ParticleManager[1].pParticleTemplate, 0u);
}

TEST_F(ZParticleEmitterInitTest, GlobalAutoStartHidesAndSchedulesWithStartDelay)
{
    BindTemplate();
    Emitter->m_bAutoStart = Emitter->m_bGlobal = Emitter->m_bHide = true;
    Emitter->m_bEmitted = true;
    Emitter->m_fStartDelay = 0.125f;
    Emitter->m_vOldParticlePos = ZVector3(1.0f, 2.0f, 3.0f);
    Emitter->m_vExplosionSplash = ZVector3(4.0f, 5.0f, 6.0f);
    SysInterface().FrameTime.secs = 2048;

    Emitter->ClassInit2();

    EXPECT_TRUE(Emitter->GetActive());
    EXPECT_TRUE(Emitter->BaseGeom()->IsHidden());
    EXPECT_FALSE(Emitter->m_bEmitted);
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 2176);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 2176);
    EXPECT_FLOAT_EQ(Emitter->m_fLoopActivate, 2.0f);
    EXPECT_EQ(Scheduled->EnableRequests, 1u);
    EXPECT_EQ(Scheduled->DisableRequests, 0u);
    ExpectVector(Emitter->m_vOldParticlePos, 0.0f, 0.0f, 0.0f);
    // Unlike the activation command, FinishInit does not clear the splash vector.
    ExpectVector(Emitter->m_vExplosionSplash, 4.0f, 5.0f, 6.0f);
}

TEST_F(ZParticleEmitterInitTest, LocalAutoStartWaitsForViewCommandEvenWhenInView)
{
    BindTemplate();
    Emitter->m_bAutoStart = true;
    Emitter->BaseGeom()->SetControlDirect(ZCINVIEW, 0);
    Emitter->ClassInit2();
    EXPECT_TRUE(Emitter->GetActive());
    EXPECT_EQ(Scheduled->EnableRequests, 0u);

    Emitter->ClassCommand(2054, nullptr);
    EXPECT_EQ(Scheduled->EnableRequests, 1u);
    Emitter->m_vOldParticlePos = ZVector3(1.0f, 2.0f, 3.0f);
    Emitter->ClassCommand(2053, nullptr);
    EXPECT_EQ(Scheduled->DisableRequests, 1u);
    EXPECT_TRUE(Emitter->GetActive());
    ExpectVector(Emitter->m_vOldParticlePos, 0.0f, 0.0f, 0.0f);
    Emitter->ClassCommand(static_cast<ZMSGID>(ZParticleEmitter::s_msgDeactivate), nullptr);
    EXPECT_EQ(Scheduled->DisableRequests, 2u);
    EXPECT_FALSE(Emitter->GetActive());
    Emitter->ClassCommand(2054, nullptr);
    EXPECT_EQ(Scheduled->EnableRequests, 1u);
}

TEST_F(ZParticleEmitterInitTest, ActivationSchedulesOnlyGlobalOrVisibleEmitters)
{
    Emitter->ClassCommand(Emitter->m_msgActivate, nullptr);
    EXPECT_EQ(Scheduled->EnableRequests, 0u);
    Emitter->BaseGeom()->SetControlDirect(ZCINVIEW, 0);
    Emitter->ClassCommand(Emitter->m_msgActivate, nullptr);
    EXPECT_EQ(Scheduled->EnableRequests, 1u);
    Emitter->BaseGeom()->SetControlDirect(0, ZCINVIEW);
    Emitter->m_bGlobal = true;
    Emitter->ClassCommand(Emitter->m_msgActivate, nullptr);
    EXPECT_EQ(Scheduled->EnableRequests, 2u);
    Emitter->ClassCommand(2053, nullptr);
    Emitter->ClassCommand(2054, nullptr);
    EXPECT_EQ(Scheduled->EnableRequests, 2u);
    EXPECT_EQ(Scheduled->DisableRequests, 0u);
}

TEST_F(ZParticleEmitterInitTest, FinishInitExpandsBoundsAroundParentCenter)
{
    BindTemplate();
    Parent = ZUniMemory::New<ZGROUP>("bounds-parent", MakeBase());
    // These pool-zeroed fields are not initialized by the group constructor.
    Parent->m_pGroupFirst = Parent->m_pGroupLast = nullptr;
    Parent->m_LightList = 0;
    Parent->m_NrAttachGeom = 0;
    Parent->BaseGeom()->m_vPos = ZVector3(10.0f, 20.0f, 30.0f);
    Parent->BaseGeom()->m_vCen = ZVector3(5.0f, 7.0f, 9.0f);
    Parent->BaseGeom()->m_vSize = ZVector3(2.0f, 3.0f, 6.0f);
    Emitter->BaseGeom()->SetParent(Parent->BaseGeom());
    Emitter->BaseGeom()->m_vPos = ZVector3(1.0f, 2.0f, 3.0f);
    Emitter->m_bExpandBounds = true;

    Emitter->ClassInit2();

    EXPECT_FLOAT_EQ(Emitter->BaseGeom()->Radius(), 8.0f);
    ExpectVector(Emitter->BaseGeom()->Size(), 2.0f, 3.0f, 6.0f);
    ExpectVector(Emitter->BaseGeom()->Cen(), 4.0f, 5.0f, 6.0f);
    EXPECT_FALSE(Emitter->GetActive());
}

TEST_F(ZParticleEmitterInitTest, TemplateNameLookupUsesRegistryAndPreservesExistingBinding)
{
    GameData = ZUniMemory::New<TemplateGameData>();
    g_pGameData = GameData;
    FirstTemplate = ZUniMemory::New<ZParticleTemplate>("first-template", MakeBase());
    NextTemplate = ZUniMemory::New<ZParticleTemplate>("next-template", MakeBase());
    GameData->m_pParticleTemplates->Add(FirstTemplate->GetRef());
    GameData->m_pParticleTemplates->Add(NextTemplate->GetRef());

    Emitter->SetszParticleTemplateName(ZRTString(""));
    EXPECT_EQ(Emitter->m_rParticleTemplate, 0u);
    Emitter->SetszParticleTemplateName(ZRTString("missing-template"));
    EXPECT_EQ(Emitter->m_rParticleTemplate, 0u);
    Emitter->SetszParticleTemplateName(ZRTString("NEXT-TEMPLATE"));
    EXPECT_EQ(Emitter->m_rParticleTemplate, 0u);
    Emitter->SetszParticleTemplateName(ZRTString("next-template"));
    EXPECT_EQ(Emitter->m_rParticleTemplate, NextTemplate->GetRef());
    Emitter->SetszParticleTemplateName(ZRTString("first-template"));
    EXPECT_EQ(Emitter->m_rParticleTemplate, NextTemplate->GetRef());
    Emitter->SetszParticleTemplateName(ZRTString(""));
    EXPECT_EQ(Emitter->m_rParticleTemplate, NextTemplate->GetRef());
    EXPECT_FALSE(Emitter->GetActive());
    EXPECT_EQ(Scheduled->EnableRequests, 0u);
}

TEST_F(ZParticleEmitterLoopTest, LoopRestartsAfterStrictDeadlineAndExpiresOnLastRestart)
{
    CaptureParticles();
    Emitter->m_bLoop = Emitter->m_bGlobal = true;
    Emitter->m_iLoopCount = 2;
    Emitter->m_fLoopInterval = 1.0f;
    Emitter->m_fLoopActivate = 0.0f;
    Emitter->m_fActivationTime.secs = 2048;
    SysInterface().FrameTime.secs = 1024;
    Emitter->ClassFrameUpdate();
    EXPECT_EQ(Scheduled->RestartRequests, 0u);
    EXPECT_EQ(Emitter->m_iLoopCount, 2);
    EXPECT_EQ(Controller->Count, 0u);

    Emitter->m_bEmitted = true;
    SysInterface().FrameTime.secs = 1025;
    Emitter->ClassFrameUpdate();
    EXPECT_EQ(Scheduled->RestartRequests, 1u);
    EXPECT_EQ(Scheduled->EnableRequests, 1u);
    EXPECT_EQ(Emitter->m_iLoopCount, 1);
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 1025);
    EXPECT_FLOAT_EQ(Emitter->m_fLoopActivate, 1025.0f / 1024.0f);
    EXPECT_TRUE(Emitter->GetActive());
    ASSERT_EQ(Controller->Count, 1u);
    EXPECT_EQ(Controller->Emissions[0].Time.secs, 1025);

    SysInterface().FrameTime.secs = 2050;
    Emitter->ClassFrameUpdate();
    EXPECT_EQ(Scheduled->RestartRequests, 2u);
    EXPECT_EQ(Scheduled->EnableRequests, 2u);
    EXPECT_EQ(Scheduled->DisableRequests, 1u);
    EXPECT_EQ(Emitter->m_iLoopCount, 0);
    EXPECT_FALSE(Emitter->GetActive());
    EXPECT_FALSE(Emitter->m_bEmitted);
    EXPECT_EQ(Controller->Count, 1u);
    EXPECT_EQ(Emitter->m_fEmitterTime.secs, 2050);
}

TEST_F(ZParticleEmitterLoopTest, LastLoopWithStartDelayDefersExpiryUntilActivationTime)
{
    CaptureParticles();
    Emitter->m_bLoop = Emitter->m_bGlobal = true;
    Emitter->m_iLoopCount = 1;
    Emitter->m_fLoopInterval = 1.0f;
    Emitter->m_fStartDelay = 0.125f;
    SysInterface().FrameTime.secs = 1025;
    Emitter->ClassFrameUpdate();
    EXPECT_EQ(Emitter->m_iLoopCount, 0);
    EXPECT_EQ(Emitter->m_fActivationTime.secs, 1153);
    EXPECT_TRUE(Emitter->GetActive());
    EXPECT_EQ(Scheduled->RestartRequests, 1u);
    EXPECT_EQ(Scheduled->EnableRequests, 1u);
    EXPECT_EQ(Scheduled->DisableRequests, 0u);

    SysInterface().FrameTime.secs = 1152;
    Emitter->ClassFrameUpdate();
    EXPECT_TRUE(Emitter->GetActive());
    EXPECT_EQ(Scheduled->DisableRequests, 0u);
    SysInterface().FrameTime.secs = 1153;
    Emitter->ClassFrameUpdate();
    EXPECT_FALSE(Emitter->GetActive());
    EXPECT_EQ(Scheduled->DisableRequests, 1u);
    EXPECT_EQ(Scheduled->RestartRequests, 1u);
    EXPECT_EQ(Controller->Count, 0u);
}
