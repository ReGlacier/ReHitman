#include <Tests/EngineFixture.h>

#include <Glacier/Action/ActionInterface.h>
#include <Glacier/Action/ZActionManager.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZInputDevice.h>
#include <Glacier/Input/ZSysInput.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace Glacier;

namespace
{
    class TestInputDevice : public ZInputDevice
    {
    public:
        TestInputDevice(SysInput::EDeviceType type, int digitalCount, int analogCount)
            : ZInputDevice(type, digitalCount, analogCount)
        {
            name[0] = '\0';
        }

        void SetName(const char* value)
        {
            std::strncpy(name, value, sizeof(name));
            name[sizeof(name) - 1] = '\0';
        }

        void SetDigitalInfo(CtrlInfo* info)
        {
            m_diginf = info;
        }

        void SetAnalogInfo(CtrlInfo* info)
        {
            m_anainf = info;
        }

        bool Connected() override
        {
            return true;
        }
    };

    class TestSysInput : public ZSysInput
    {
    public:
        void SetActiveController(int) override
        {
        }

        uint8_t SetTimeoutCallback(SysInput::TimeoutCallback, int) override
        {
            return 0;
        }

        void UnSetTimeoutCallback() override
        {
        }

        bool IsControllerConnected() override
        {
            return false;
        }
    };

    class ActionBindingTest : public Tests::EngineFixture
    {
    protected:
        void SetUp() override
        {
            Tests::EngineFixture::SetUp();

            m_PreviousSysInput = SysInput::instance;
            m_PreviousAction = Action::instance;

            auto* input = ZUniMemory::New<TestSysInput>();
            auto* keyboard = ZUniMemory::New<TestInputDevice>(SysInput::eKEYBOARD_TYPE, 4, 2);

            keyboard->SetName("Keyboard Device");
            keyboard->SetDigitalInfo(m_DigitalInfo);
            keyboard->SetAnalogInfo(m_AnalogInfo);

            ASSERT_EQ(input->AddDevice(keyboard), 1);

            m_Keyboard = keyboard;
            SysInput::instance = input;
            Action::instance = nullptr;
        }

        void TearDown() override
        {
            if (Action::instance)
            {
                ZUniMemory::Delete(Action::instance);
                Action::instance = nullptr;
            }

            if (SysInput::instance && SysInput::instance != m_PreviousSysInput)
            {
                ZUniMemory::Delete(SysInput::instance);
            }

            SysInput::instance = m_PreviousSysInput;
            Action::instance = m_PreviousAction;

            Tests::EngineFixture::TearDown();
        }

        TestInputDevice& Keyboard()
        {
            return *m_Keyboard;
        }

        ZInputDevice::CtrlInfo m_DigitalInfo[5]
        {
            { "k", 0 },
            { "p", 1 },
            { "z", 2 },
            { "x", 3 },
            {}
        };

        ZInputDevice::CtrlInfo m_AnalogInfo[3]
        {
            { "axis", 0 },
            { "wheel", 1 },
            {}
        };

    private:
        SysInput::ZInterface* m_PreviousSysInput{};
        ZActionManager* m_PreviousAction{};
        TestInputDevice* m_Keyboard{};
    };
}

TEST_F(ActionBindingTest, ParsesGameStyleDebugBlock)
{
    ZActionManager manager;

    ASSERT_TRUE(manager.AddBindings(R"(
Debug={
    ToggleFreeCam=tap(kb,k);
    Pause=tap(kb,p);
    EndDeathSequence=tap(kb,z);
    CycleInventory=tap(kb,x);
};
)"));

    auto* debug = manager.GetMapping("Debug");
    ASSERT_NE(debug, nullptr);
    EXPECT_STREQ(debug->GetName(), "Debug");

    auto* toggleFreeCam = manager.GetMapping("ToggleFreeCam");
    ASSERT_NE(toggleFreeCam, nullptr);
    EXPECT_EQ(toggleFreeCam->m_eType, ZActionMapTree::eTAP);
    EXPECT_EQ(static_cast<int16_t>(toggleFreeCam->m_iDeviceId), 0);
    EXPECT_EQ(static_cast<int16_t>(toggleFreeCam->m_iControlId), 0);
    EXPECT_STREQ(toggleFreeCam->GetKeyName(), "k");

    auto* pause = manager.GetMapping("Pause");
    ASSERT_NE(pause, nullptr);
    EXPECT_EQ(pause->m_eType, ZActionMapTree::eTAP);
    EXPECT_EQ(static_cast<int16_t>(pause->m_iDeviceId), 0);
    EXPECT_EQ(static_cast<int16_t>(pause->m_iControlId), 1);
    EXPECT_STREQ(pause->GetKeyName(), "p");

    auto* endDeathSequence = manager.GetMapping("EndDeathSequence");
    ASSERT_NE(endDeathSequence, nullptr);
    EXPECT_EQ(static_cast<int16_t>(endDeathSequence->m_iControlId), 2);

    auto* cycleInventory = manager.GetMapping("CycleInventory");
    ASSERT_NE(cycleInventory, nullptr);
    EXPECT_EQ(static_cast<int16_t>(cycleInventory->m_iControlId), 3);

    ASSERT_EQ(debug->FirstChild(), toggleFreeCam);
    EXPECT_EQ(toggleFreeCam->Next(), pause);
    EXPECT_EQ(pause->Next(), endDeathSequence);
    EXPECT_EQ(endDeathSequence->Next(), cycleInventory);
    EXPECT_EQ(cycleInventory->Next(), nullptr);
}

TEST_F(ActionBindingTest, ParsesGetReference)
{
    ZActionManager manager;

    ASSERT_TRUE(manager.AddBindings("Pause=tap(kb,p);PauseAlias=get(Pause);"));

    auto* pause = manager.GetMapping("Pause");
    auto* pauseAlias = manager.GetMapping("PauseAlias");

    ASSERT_NE(pause, nullptr);
    ASSERT_NE(pauseAlias, nullptr);
    EXPECT_EQ(pauseAlias->m_eType, ZActionMapTree::eGET);
    EXPECT_EQ(pauseAlias->m_pkGetTree, pause);
}

TEST_F(ActionBindingTest, HandleRefreshesAfterBindingsChange)
{
    auto* manager = ZUniMemory::New<ZActionManager>();
    Action::instance = manager;

    Action::ZHandle handle("Pause");
    EXPECT_FALSE(handle.CheckMap());

    ASSERT_TRUE(manager->AddBindings("Pause=tap(kb,p);"));

    EXPECT_TRUE(handle.CheckMap());
    ASSERT_NE(handle.m_pkMap, nullptr);
    EXPECT_STREQ(handle.m_pkMap->GetKeyName(), "p");
}

TEST_F(ActionBindingTest, HandleActivatedByForwardsToMappedTree)
{
    auto* manager = ZUniMemory::New<ZActionManager>();
    Action::instance = manager;

    ASSERT_TRUE(manager->AddBindings("Pause=tap(kb,p);"));

    Action::ZHandle handle("Pause");
    ASSERT_TRUE(handle.CheckMap());

    handle.m_pkMap->m_iLastDevice = 7;

    EXPECT_EQ(handle.ActivatedBy(), 7);
}

TEST_F(ActionBindingTest, DigitalEvaluatesHoldAndTap)
{
    auto* manager = ZUniMemory::New<ZActionManager>();
    Action::instance = manager;

    ASSERT_TRUE(manager->AddBindings("HoldK=hold(kb,k);TapP=tap(kb,p);"));

    auto* holdK = manager->GetMapping("HoldK");
    auto* tapP = manager->GetMapping("TapP");
    ASSERT_NE(holdK, nullptr);
    ASSERT_NE(tapP, nullptr);

    SysInterface().m_fRealTime = TIMETYPE(1.0f);

    EXPECT_FALSE(holdK->Digital());

    Keyboard().buttonSample(0, true, TIMETYPE(1.0f));
    EXPECT_TRUE(holdK->Digital());
    EXPECT_EQ(holdK->ActivatedBy(), 0);

    SysInterface().m_fRealTime = TIMETYPE(1.1f);
    Keyboard().buttonSample(1, true, TIMETYPE(1.1f));
    EXPECT_TRUE(tapP->Digital());

    EXPECT_FALSE(tapP->Digital());
}

TEST_F(ActionBindingTest, AnalogEvaluatesAbsoluteRelativeAndArithmetic)
{
    auto* manager = ZUniMemory::New<ZActionManager>();
    Action::instance = manager;

    ASSERT_TRUE(manager->AddBindings("Axis=ana(kb,axis);Wheel=rel(kb,wheel);AxisPlus=+ ana(kb,axis) 2.0;"));

    auto* axis = manager->GetMapping("Axis");
    auto* wheel = manager->GetMapping("Wheel");
    auto* axisPlus = manager->GetMapping("AxisPlus");
    ASSERT_NE(axis, nullptr);
    ASSERT_NE(wheel, nullptr);
    ASSERT_NE(axisPlus, nullptr);

    Keyboard().AnalogSample(0, 0.25f);
    EXPECT_FLOAT_EQ(axis->Analog(), 0.25f);

    Keyboard().AnalogMotionSample(1, 1.5f);
    EXPECT_FLOAT_EQ(wheel->Analog(), 1.5f);

    EXPECT_FLOAT_EQ(axisPlus->Analog(), 2.25f);
}

TEST_F(ActionBindingTest, ClearInputKeysClearsDigitalStateAndHistory)
{
    ZActionManager manager;
    ASSERT_TRUE(manager.AddBindings("HoldK=hold(kb,k);"));

    auto* holdK = manager.GetMapping("HoldK");
    ASSERT_NE(holdK, nullptr);

    Keyboard().buttonSample(0, true, TIMETYPE(1.0f));
    ASSERT_EQ(Keyboard().DigitalState(0), 1);
    ASSERT_NE(Keyboard().DigitalHist(0, 0).secs, 0);

    holdK->ClearInputKeys();

    EXPECT_EQ(Keyboard().DigitalState(0), 0);
    EXPECT_EQ(Keyboard().DigitalHist(0, 0).secs, 0);
    EXPECT_EQ(Keyboard().DigitalHist(0, 1).secs, 0);
}
