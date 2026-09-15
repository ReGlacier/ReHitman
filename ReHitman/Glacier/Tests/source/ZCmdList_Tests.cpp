#include <Glacier/Render/Cmd/ZCmdList.h>

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>

using namespace Glacier;

namespace
{
    struct CmdListFixture
    {
        std::array<char, 256> Buffer {};
        ZCmdList CmdList {};

        CmdListFixture()
        {
            CmdList.m_lPreTransactionNrCmds = 0;
            CmdList.m_pPreTransactionCurrent = nullptr;
            CmdList.m_lNrCmds = 0;
            CmdList.m_pCurrent = Buffer.data();
            CmdList.m_pBufferEnd = Buffer.data() + Buffer.size();
            CmdList.m_Buffer = Buffer.data();
        }
    };
}

TEST(ZCmdList, LayoutMatchesPcInstance)
{
    static_assert(sizeof(ZCmdList) == 0x18);
    static_assert(sizeof(ZCmdList::ZCmd) == 0x18);
}

TEST(ZCmdList, CurrentReturnsCommandAtCurrentBufferPointer)
{
    CmdListFixture fixture;

    EXPECT_EQ(fixture.CmdList.Current(), reinterpret_cast<ZCmdList::ZCmd*>(fixture.Buffer.data()));
}

TEST(ZCmdList, NextCommandAdvancesByCommandHeaderAndIncrementsCount)
{
    CmdListFixture fixture;
    ZCmdList::ZCmd* pFirst = fixture.CmdList.Current();

    fixture.CmdList.NextCommand();

    EXPECT_EQ(fixture.CmdList.m_lNrCmds, 1u);
    EXPECT_EQ(fixture.CmdList.Current(), pFirst + 1);
}

TEST(ZCmdList, AddDataReservesAlignedPayloadAndTracksDwordCount)
{
    CmdListFixture fixture;
    ZCmdList::ZCmd* pCmd = fixture.CmdList.Current();
    pCmd->m_pCmdList = &fixture.CmdList;
    pCmd->m_lNrObjects = 0;
    fixture.CmdList.NextCommand();

    void* pPayload = pCmd->AddData(16);

    EXPECT_EQ(pPayload, reinterpret_cast<void*>(pCmd + 1));
    EXPECT_EQ(fixture.CmdList.m_pCurrent, reinterpret_cast<char*>(pCmd + 1) + 16);
    EXPECT_EQ(pCmd->m_lNrObjects, 4u);
}

TEST(ZCmdList, AddDataCopiesPayload)
{
    CmdListFixture fixture;
    ZCmdList::ZCmd* pCmd = fixture.CmdList.Current();
    pCmd->m_pCmdList = &fixture.CmdList;
    pCmd->m_lNrObjects = 0;
    fixture.CmdList.NextCommand();

    const uint32_t payload[] = { 0x11223344u, 0x55667788u, 0x99AABBCCu };
    void* pCopied = pCmd->AddData(const_cast<uint32_t*>(payload), sizeof(payload));

    EXPECT_EQ(pCopied, reinterpret_cast<void*>(pCmd + 1));
    EXPECT_EQ(std::memcmp(pCopied, payload, sizeof(payload)), 0);
    EXPECT_EQ(fixture.CmdList.m_pCurrent, reinterpret_cast<char*>(pCmd + 1) + sizeof(payload));
    EXPECT_EQ(pCmd->m_lNrObjects, 3u);
}

TEST(ZCmdList, CommandNextSkipsPayloadBytes)
{
    CmdListFixture fixture;
    ZCmdList::ZCmd* pCmd = fixture.CmdList.Current();
    pCmd->m_pCmdList = &fixture.CmdList;
    pCmd->m_lNrObjects = 0;
    fixture.CmdList.NextCommand();

    pCmd->AddData(16);

    EXPECT_EQ(pCmd->Next(), reinterpret_cast<ZCmdList::ZCmd*>(reinterpret_cast<char*>(pCmd + 1) + 16));
}

TEST(ZCmdList, CommandNextReturnsAdjacentCommandWithoutPayload)
{
    CmdListFixture fixture;
    ZCmdList::ZCmd* pCmd = fixture.CmdList.Current();
    pCmd->m_lNrObjects = 0;

    EXPECT_EQ(pCmd->Next(), pCmd + 1);
}
