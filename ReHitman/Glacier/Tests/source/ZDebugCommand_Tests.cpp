#include <Glacier/Debug/ZDebugCommand.h>
#include <Glacier/Debug/ZDebugIntRef.h>
#include <Glacier/Debug/ZDebugVarRef.h>

#include <gtest/gtest.h>

#include <cstring>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZDebugCommand) == 0x24);
    static_assert(sizeof(ZDebugIntRef) == 0x34);

    class ZTestCommand : public ZDebugCommand
    {
    public:
        ZTestCommand(const char* pszName, const char* pszDescription)
            : ZDebugCommand(pszName, pszDescription)
        {
        }

        void Execute(uint32_t iArgC, const char** ppszArgV) override
        {
            ++m_iExecuteCount;
        }

        int m_iExecuteCount = 0;
    };

    int CountRegisteredCommands()
    {
        int iCount = 0;
        for (ZDebugCommand* pCmd = ZDebugCommand::First(); pCmd; pCmd = pCmd->Next())
            ++iCount;
        return iCount;
    }

    ZDebugCommand* FindCommand(const char* pszName)
    {
        for (ZDebugCommand* pCmd = ZDebugCommand::First(); pCmd; pCmd = pCmd->Next())
        {
            if (std::strcmp(pCmd->Name(), pszName) == 0)
                return pCmd;
        }
        return nullptr;
    }
}

TEST(ZDebugCommand, ConstructionAppendsToGlobalRegistry)
{
    const int iBase = CountRegisteredCommands();

    {
        ZTestCommand cmd("rehitman_test_cmd_a", "test command A");

        EXPECT_EQ(CountRegisteredCommands(), iBase + 1);
        EXPECT_EQ(FindCommand("rehitman_test_cmd_a"), &cmd);
        EXPECT_STREQ(cmd.Name(), "rehitman_test_cmd_a");
        EXPECT_STREQ(cmd.Description(), "test command A");
        EXPECT_EQ(cmd.GetType(), ECLASS_UNKNOWN);
    }

    EXPECT_EQ(CountRegisteredCommands(), iBase);
    EXPECT_EQ(FindCommand("rehitman_test_cmd_a"), nullptr);
}

TEST(ZDebugCommand, RegistryKeepsConstructionOrder)
{
    ZTestCommand cmdA("rehitman_test_ord_a", "A");
    ZTestCommand cmdB("rehitman_test_ord_b", "B");
    ZTestCommand cmdC("rehitman_test_ord_c", "C");

    ASSERT_EQ(cmdA.Next(), &cmdB);
    ASSERT_EQ(cmdB.Next(), &cmdC);
    EXPECT_EQ(cmdC.Next(), nullptr);
}

TEST(ZDebugCommand, DestroyingElementRelinksList)
{
    ZTestCommand cmdA("rehitman_test_link_a", "A");
    ZTestCommand cmdC("rehitman_test_link_c", "C");

    {
        ZTestCommand cmdB("rehitman_test_link_b", "B");
        ASSERT_EQ(cmdA.Next(), &cmdC);
        ASSERT_EQ(cmdC.Next(), &cmdB);
        EXPECT_EQ(cmdB.Next(), nullptr);
    }

    ASSERT_EQ(cmdA.Next(), &cmdC);
    EXPECT_EQ(cmdC.Next(), nullptr);
}

TEST(ZDebugCommand, GetValuePointsAtInlineBuffer)
{
    ZTestCommand cmd("rehitman_test_value", "value");

    EXPECT_EQ(cmd.GetValue(), cmd.m_szValue);
}

TEST(ZDebugIntRef, RegistersItselfAndReportsIntType)
{
    int32_t iValue = 0;

    {
        ZDebugIntRef var(iValue, "rehitman_test_int", "test int", -1000000, 1000000, 1, nullptr);

        EXPECT_EQ(FindCommand("rehitman_test_int"), &var);
        EXPECT_EQ(var.GetType(), ECLASS_INT);
        EXPECT_EQ(static_cast<ZDebugCommand&>(var).GetPath(), nullptr);
    }

    EXPECT_EQ(FindCommand("rehitman_test_int"), nullptr);
}

TEST(ZDebugIntRef, IncDecClampToMinMax)
{
    int32_t iValue = 0;
    ZDebugIntRef var(iValue, "rehitman_test_clamp", "clamp", -2, 2, 1, nullptr);

    var.Inc();
    var.Inc();
    var.Inc();
    EXPECT_EQ(iValue, 2); // clamped at max

    var.Dec();
    var.Dec();
    var.Dec();
    var.Dec();
    var.Dec();
    EXPECT_EQ(iValue, -2); // clamped at min
}

TEST(ZDebugIntRef, ExecuteWithArgumentAssignsValue)
{
    int32_t iValue = 5;
    ZDebugIntRef var(iValue, "rehitman_test_exec", "exec", -1000000, 1000000, 1, nullptr);

    const char* apszArgs[] = { "rehitman_test_exec", "42" };
    var.Execute(2, apszArgs);

    EXPECT_EQ(iValue, 42);
}

TEST(ZDebugIntRef, ExecuteWithoutArgumentOnlyPrints)
{
    int32_t iValue = 7;
    ZDebugIntRef var(iValue, "rehitman_test_print", "print", -1000000, 1000000, 1, nullptr);

    const char* apszArgs[] = { "rehitman_test_print" };
    var.Execute(1, apszArgs);

    EXPECT_EQ(iValue, 7);
}

TEST(ZDebugIntRef, CalcValueFormatsIntoInlineBuffer)
{
    int32_t iValue = -123;
    ZDebugIntRef var(iValue, "rehitman_test_calc", "calc", -1000000, 1000000, 1, nullptr);

    var.CalcValue();

    EXPECT_STREQ(var.GetValue(), "-123");
}

TEST(ZDebugVarRef, AssignmentOperatorWritesThroughReference)
{
    int32_t iValue = 1;
    ZDebugIntRef var(iValue, "rehitman_test_assign", "assign", -100, 100, 1, nullptr);

    ZDebugVarRef<int32_t>& ref = var;
    ref = 55;

    EXPECT_EQ(iValue, 55);
    EXPECT_EQ(static_cast<int32_t>(var), 55);
}
