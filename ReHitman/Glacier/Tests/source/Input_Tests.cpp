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
}

TEST(InputDevice, ControlNamesUseSentinelTerminatedTables)
{
    TestInputDevice device(SysInput::eKEYBOARD_TYPE, 4, 2);
    ZInputDevice::CtrlInfo digital[] = { { "fire", 1 }, { "jump", 3 }, {} };
    ZInputDevice::CtrlInfo analog[] = { { "x", 0 }, { "y", 1 }, {} };

    device.SetDigitalInfo(digital);
    device.SetAnalogInfo(analog);

    EXPECT_STREQ(device.DigitalName(1), "fire");
    EXPECT_STREQ(device.DigitalName(3), "jump");
    EXPECT_EQ(device.DigitalName(2), nullptr);
    EXPECT_EQ(device.DigitalId("jump"), 3);
    EXPECT_EQ(device.DigitalId("missing"), -1);

    EXPECT_STREQ(device.AnalogName(0), "x");
    EXPECT_STREQ(device.AnalogName(1), "y");
    EXPECT_EQ(device.AnalogName(2), nullptr);
    EXPECT_EQ(device.AnalogId("y"), 1);
    EXPECT_EQ(device.AnalogId("missing"), -1);
}

TEST(InputDevice, DigitalSamplesTrackStateAndHistory)
{
    TestInputDevice device(SysInput::eMOUSE_TYPE, 2, 0);

    device.buttonSample(1, true, TIMETYPE(1));
    EXPECT_EQ(device.DigitalState(1), 1);
    EXPECT_EQ(device.DigitalHist(1, 0).secs, TIMETYPE(1).secs);
    EXPECT_EQ(device.DigitalHist(1, 1).secs, 0);

    device.buttonSample(1, true, TIMETYPE(2));
    EXPECT_EQ(device.DigitalHist(1, 0).secs, TIMETYPE(1).secs);

    device.buttonSample(1, false, TIMETYPE(3));
    EXPECT_EQ(device.DigitalState(1), 0);
    EXPECT_EQ(device.DigitalHist(1, 0).secs, TIMETYPE(3).secs);
    EXPECT_EQ(device.DigitalHist(1, 1).secs, TIMETYPE(1).secs);
}

TEST(InputDevice, AnalogSamplingSupportsAbsoluteAndRelativeMotion)
{
    TestInputDevice device(SysInput::eGAMECONTROL_TYPE, 0, 2);

    device.AnalogSample(0, 0.25f);
    EXPECT_FLOAT_EQ(device.AnalogState(0), 0.25f);
    EXPECT_FLOAT_EQ(device.AnalogMotion(0), 0.25f);

    device.AnalogSample(0, 0.75f);
    EXPECT_FLOAT_EQ(device.AnalogState(0), 0.75f);
    EXPECT_FLOAT_EQ(device.AnalogMotion(0), 0.5f);

    device.AnalogMotionSample(1, 2.0f);
    device.AnalogMotionSample(1, -0.5f);
    EXPECT_FLOAT_EQ(device.AnalogState(1), -0.5f);
    EXPECT_FLOAT_EQ(device.AnalogMotion(1), 1.5f);
}

TEST(InputDevice, ResetTablesClearsDigitalHistoryAndAnalogState)
{
    TestInputDevice device(SysInput::eGAMECONTROL_TYPE, 2, 2);

    device.buttonSample(0, true, TIMETYPE(1));
    device.buttonSample(0, false, TIMETYPE(2));
    device.AnalogSample(0, 0.75f);
    device.AnalogMotionSample(1, 1.5f);

    device.ResetTables(true);

    EXPECT_EQ(device.DigitalState(0), 0);
    EXPECT_EQ(device.DigitalHist(0, 0).secs, 0);
    EXPECT_EQ(device.DigitalHist(0, 1).secs, 0);
    EXPECT_FLOAT_EQ(device.AnalogState(0), 0.0f);
    EXPECT_FLOAT_EQ(device.AnalogMotion(0), 0.0f);
    EXPECT_FLOAT_EQ(device.AnalogState(1), 0.0f);
    EXPECT_FLOAT_EQ(device.AnalogMotion(1), 0.0f);
}

TEST(SysInput, DeviceLookupAndNaming)
{
    TestSysInput input;
    auto* mouse = ZUniMemory::New<TestInputDevice>(SysInput::eMOUSE_TYPE, 1, 0);
    auto* keyboard = ZUniMemory::New<TestInputDevice>(SysInput::eKEYBOARD_TYPE, 1, 0);
    auto* pad0 = ZUniMemory::New<TestInputDevice>(SysInput::eGAMECONTROL_TYPE, 1, 0);
    auto* pad1 = ZUniMemory::New<TestInputDevice>(SysInput::eGAMECONTROL_TYPE, 1, 0);

    mouse->SetName("Mouse Device");
    keyboard->SetName("Keyboard Device");
    pad0->SetName("Pad Zero");
    pad1->SetName("Pad One");

    ASSERT_EQ(input.AddDevice(mouse), 1);
    ASSERT_EQ(input.AddDevice(keyboard), 1);
    ASSERT_EQ(input.AddDevice(pad0), 1);
    ASSERT_EQ(input.AddDevice(pad1), 1);

    EXPECT_STREQ(input.GetDeviceName(0), "ms");
    EXPECT_STREQ(input.GetDeviceName(1), "kb");
    EXPECT_STREQ(input.GetDeviceName(2), "gc");
    EXPECT_STREQ(input.GetDeviceName(3), "gc1");

    EXPECT_EQ(input.GetDeviceIdByName("mouse"), 0);
    EXPECT_EQ(input.GetDeviceIdByName("keyboard"), 1);
    EXPECT_EQ(input.GetDeviceIdByName("gc"), 2);
    EXPECT_EQ(input.GetDeviceIdByName("gc1"), 3);
    EXPECT_EQ(input.GetDeviceIdByName("Pad One"), 3);
    EXPECT_EQ(input.GetNthDevice(SysInput::eGAMECONTROL_TYPE, 1), 3);
    EXPECT_EQ(input.GetDeviceIdByPtr(pad0), 2);

    ASSERT_EQ(input.DeleteDevice(keyboard), 1);
    ZUniMemory::Delete(keyboard);
    EXPECT_EQ(input.GetDeviceIdByPtr(pad0), 1);
    EXPECT_EQ(pad0->m_iDeviceID, 1);
}
