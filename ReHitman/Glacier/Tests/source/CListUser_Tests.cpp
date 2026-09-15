#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/ZSTL/CListUser.h>
#include <Glacier/ZUniMemory.h>
#include <Tests/EngineFixture.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>

using namespace Glacier;

namespace
{
    constexpr uint32_t kStaticWords = 16;
    constexpr uint32_t kDynamicWords = 16;
    constexpr uint32_t kRuntimeWords = 8;
    constexpr uint32_t kDirtyBit = 0x01000000u;
    constexpr uint32_t kListEnd = 1u;
    constexpr uint32_t kListRange = 2u;
    constexpr uint32_t kListAttribs = 7u;
    constexpr uint32_t kListValueMask = 0xFFFFFFF8u;

    uint32_t Pack(void* pNode, uint32_t attribs = 0)
    {
        return (reinterpret_cast<uint32_t>(pNode) & kListValueMask) | (attribs & kListAttribs);
    }

    uint32_t& RawListWord(ZBaseGeom* geom)
    {
        return *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(geom) + 0x54);
    }

    struct CListUserTest : Tests::EngineFixture
    {
        ZGeomBuffer GeomBuffer{ sizeof(ZBaseGeom) * 8, 256, 512, 128 };
        std::array<uint32_t, kStaticWords + kRuntimeWords + kDynamicWords> Buffer{};

        CListUserTest()
        {
            Buffer[0] = kStaticWords;
            Buffer[1] = kDynamicWords;
            Buffer[2] = kRuntimeWords;
        }

        void Cleanup(CListUser& list)
        {
            if (list.m_pGetDynEntryPool)
            {
                ZUniMemory::Delete(list.m_pGetDynEntryPool);
                list.m_pGetDynEntryPool = nullptr;
            }

            if (list.m_pCollidedWith)
            {
                ZUniMemory::Free(list.m_pCollidedWith);
                list.m_pCollidedWith = nullptr;
                list.m_pInternalListUnfold = nullptr;
                list.m_pInternalOutBuf = nullptr;
            }
        }

        ZBaseGeom* NewGeom(uint32_t listId = 0)
        {
            auto* geom = new ZBaseGeom;
            geom->m_uListID = listId;
            return geom;
        }

        void DeleteGeom(ZBaseGeom* geom)
        {
            delete geom;
        }

        void MakeSingleStaticList(uint32_t listId, ZBaseGeom* node)
        {
            Buffer[listId] = 0;
            Buffer[listId + 1] = Pack(node, kListEnd);
        }
    };
}

TEST_F(CListUserTest, ConstructorInitializesGeneratedBufferPools)
{
    CListUser list(Buffer.data());

    EXPECT_EQ(list.m_uSizeOfStaticBuf, kStaticWords);
    EXPECT_EQ(list.m_uSizeOfDynamicBuf, kDynamicWords);
    EXPECT_EQ(list.m_uSizeOfRuntimeBuf, kRuntimeWords);
    EXPECT_EQ(list.m_pStaticBuf, Buffer.data());
    EXPECT_EQ(list.GetTotalBufferSize(), kStaticWords + kDynamicWords + kRuntimeWords);
    EXPECT_EQ(list.GetFullBuffer(), Buffer.data());
    EXPECT_EQ(list.GetPoolPtr(), &Buffer[kStaticWords + kRuntimeWords]);
    EXPECT_EQ(list.m_pGetDynEntryPool->GetPoolPtr(), &Buffer[kStaticWords]);

    uint32_t maxInput = 0;
    EXPECT_EQ(list.GetCatchBuffer(&maxInput), list.m_pCollidedWith);
    EXPECT_EQ(maxInput, 1024u);

    Cleanup(list);
}

TEST_F(CListUserTest, ConnectBatchAddsNodesToDynamicBlocksAndUnfoldsThem)
{
    CListUser list(Buffer.data());
    auto* first = NewGeom(4);
    auto* second = NewGeom(4);
    auto* third = NewGeom(4);
    auto* fourth = NewGeom(4);
    uint32_t nodes[] = {
        reinterpret_cast<uint32_t>(first),
        reinterpret_cast<uint32_t>(second),
        reinterpret_cast<uint32_t>(third),
        reinterpret_cast<uint32_t>(fourth),
    };
    Buffer[5] = 1;

    EXPECT_EQ(list.ConnectBatchToList(4, nodes, 4), 4u);

    uint32_t length = 0;
    uint32_t* unfolded = list.UnfoldList(&length, 4);
    ASSERT_NE(unfolded, nullptr);
    EXPECT_EQ(length, 4u);
    EXPECT_EQ(unfolded[0], reinterpret_cast<uint32_t>(fourth));
    EXPECT_EQ(unfolded[1], reinterpret_cast<uint32_t>(third));
    EXPECT_EQ(unfolded[2], reinterpret_cast<uint32_t>(second));
    EXPECT_EQ(unfolded[3], reinterpret_cast<uint32_t>(first));

    DeleteGeom(fourth);
    DeleteGeom(third);
    DeleteGeom(second);
    DeleteGeom(first);
    Cleanup(list);
}

TEST_F(CListUserTest, DisconnectDynamicListRemovesNodeAndCompactsBlocks)
{
    CListUser list(Buffer.data());
    auto* first = NewGeom(4);
    auto* second = NewGeom(4);
    auto* third = NewGeom(4);
    uint32_t nodes[] = {
        reinterpret_cast<uint32_t>(first),
        reinterpret_cast<uint32_t>(second),
        reinterpret_cast<uint32_t>(third),
    };
    Buffer[5] = 1;

    ASSERT_EQ(list.ConnectBatchToList(4, nodes, 3), 3u);
    EXPECT_EQ(list.DisconnectFromDynamicList(4, second), true);

    uint32_t length = 0;
    uint32_t* unfolded = list.UnfoldList(&length, 4);
    ASSERT_NE(unfolded, nullptr);
    EXPECT_EQ(length, 2u);
    EXPECT_EQ(unfolded[0], reinterpret_cast<uint32_t>(third));
    EXPECT_EQ(unfolded[1], reinterpret_cast<uint32_t>(first));

    DeleteGeom(third);
    DeleteGeom(second);
    DeleteGeom(first);
    Cleanup(list);
}

TEST_F(CListUserTest, NotifyAllMembersMarksCallerAndMembersDirty)
{
    CListUser list(Buffer.data());
    auto* caller = NewGeom(4);
    auto* member = NewGeom(4);
    uint32_t nodes[] = { reinterpret_cast<uint32_t>(member) };
    Buffer[5] = 1;

    ASSERT_EQ(list.ConnectBatchToList(4, nodes, 1), 1u);
    list.NotifyAllMembers(caller);

    EXPECT_NE(caller->m_lControl & kDirtyBit, 0u);
    EXPECT_NE(member->m_lControl & kDirtyBit, 0u);

    DeleteGeom(member);
    DeleteGeom(caller);
    Cleanup(list);
}

TEST_F(CListUserTest, StaticListLookupAndEndEntryDelete)
{
    CListUser list(Buffer.data());
    auto* node = NewGeom(4);
    MakeSingleStaticList(4, node);

    EXPECT_TRUE(list.IsNodeInStaticList(4, node));
    EXPECT_TRUE(list.DisconnectFromStaticList(4, node));
    EXPECT_EQ(Buffer[5], kListEnd);
    EXPECT_FALSE(list.IsNodeInStaticList(4, node));

    DeleteGeom(node);
    Cleanup(list);
}

TEST_F(CListUserTest, StaticListNonEndDeleteCompactsEntries)
{
    CListUser list(Buffer.data());
    auto* first = NewGeom(4);
    auto* second = NewGeom(4);
    Buffer[4] = 0;
    Buffer[5] = Pack(first);
    Buffer[6] = Pack(second, kListEnd);

    EXPECT_TRUE(list.DisconnectFromStaticList(4, first));
    EXPECT_EQ(Buffer[5], Pack(second, kListEnd));
    EXPECT_TRUE(list.IsNodeInStaticList(4, second));
    EXPECT_FALSE(list.IsNodeInStaticList(4, first));

    DeleteGeom(second);
    DeleteGeom(first);
    Cleanup(list);
}

TEST_F(CListUserTest, StaticRangeUnfoldExpandsContiguousGeometryNodes)
{
    CListUser list(Buffer.data());
    auto* first = NewGeom(4);
    auto* second = NewGeom(4);
    auto* third = NewGeom(4);
    ASSERT_EQ(first + 1, second);
    ASSERT_EQ(second + 1, third);

    Buffer[4] = 0;
    Buffer[5] = Pack(first, kListRange);
    Buffer[6] = Pack(third, kListEnd);

    uint32_t length = 0;
    uint32_t* unfolded = list.UnfoldList(&length, 4);
    ASSERT_NE(unfolded, nullptr);
    EXPECT_EQ(length, 3u);
    EXPECT_EQ(unfolded[0], reinterpret_cast<uint32_t>(first));
    EXPECT_EQ(unfolded[1], reinterpret_cast<uint32_t>(second));
    EXPECT_EQ(unfolded[2], reinterpret_cast<uint32_t>(third));

    DeleteGeom(third);
    DeleteGeom(second);
    DeleteGeom(first);
    Cleanup(list);
}

TEST_F(CListUserTest, StaticRangeDeleteReconnectsRemovedSideIntoDynamicList)
{
    CListUser list(Buffer.data());
    auto* first = NewGeom(4);
    auto* second = NewGeom(4);
    auto* third = NewGeom(4);
    auto* fourth = NewGeom(4);
    ASSERT_EQ(first + 1, second);
    ASSERT_EQ(second + 1, third);
    ASSERT_EQ(third + 1, fourth);

    Buffer[4] = 0;
    Buffer[5] = Pack(first, kListRange);
    Buffer[6] = Pack(fourth, kListEnd);

    EXPECT_TRUE(list.DisconnectFromStaticList(4, second));
    EXPECT_TRUE(list.IsNodeInDynamicList(4, first));
    EXPECT_FALSE(list.IsNodeInStaticList(4, second));
    EXPECT_TRUE(list.IsNodeInStaticList(4, third));
    EXPECT_TRUE(list.IsNodeInStaticList(4, fourth));

    DeleteGeom(fourth);
    DeleteGeom(third);
    DeleteGeom(second);
    DeleteGeom(first);
    Cleanup(list);
}

TEST_F(CListUserTest, AddAndRemoveRuntimeMemberUsesRuntimePoolAndPreservesHighByte)
{
    CListUser list(Buffer.data());
    auto* node = NewGeom();

    const uint32_t listId = list.AddRuntimeMember(node);
    ASSERT_NE(listId, 0u);
    RawListWord(node) = (listId & 0xFFFFFFu) | 0xAB000000u;
    Buffer[listId + 1] = kListEnd;

    list.RemoveRuntimeMember(node);

    EXPECT_EQ(RawListWord(node), 0xABu);

    DeleteGeom(node);
    Cleanup(list);
}

TEST_F(CListUserTest, AnalyzeCatchReplacesOldMembersWithCollisionSet)
{
    CListUser list(Buffer.data());
    auto* caller = NewGeom(4);
    auto* oldMember = NewGeom(8);
    auto* newMember = NewGeom(10);
    uint32_t oldNodes[] = { reinterpret_cast<uint32_t>(oldMember) };
    Buffer[5] = kListEnd;
    Buffer[9] = kListEnd;
    Buffer[11] = kListEnd;

    ASSERT_EQ(list.ConnectBatchToList(4, oldNodes, 1), 1u);
    ASSERT_EQ(list.ConnectToList(8, caller), true);

    list.m_pCollidedWith[0] = reinterpret_cast<uint32_t>(newMember);
    list.AnalyzeCatch(1, caller);

    EXPECT_FALSE(list.IsNodeInList(4, oldMember));
    EXPECT_FALSE(list.IsNodeInList(8, caller));
    EXPECT_TRUE(list.IsNodeInList(4, newMember));
    EXPECT_TRUE(list.IsNodeInList(10, caller));
    EXPECT_NE(caller->m_lControl & kDirtyBit, 0u);
    EXPECT_NE(oldMember->m_lControl & kDirtyBit, 0u);
    EXPECT_NE(newMember->m_lControl & kDirtyBit, 0u);

    DeleteGeom(newMember);
    DeleteGeom(oldMember);
    DeleteGeom(caller);
    Cleanup(list);
}
