#include <gtest/gtest.h>
#include <Glacier/ZSTL/CHUNK.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


using namespace Glacier;

TEST(CHUNKTest, ConstructorInitializesEmptyChunk)
{
    CHUNK chunk { 0x1234 };

    EXPECT_EQ(chunk.Name, 0x1234);
    EXPECT_EQ(chunk.TotalSize, 0);
    EXPECT_EQ(chunk.HeaderSize, 0);
    EXPECT_EQ(chunk.Header, nullptr);
    EXPECT_EQ(chunk.DataListCount, 0);
    EXPECT_EQ(chunk.DataList, nullptr);
    EXPECT_EQ(chunk.DataFirst, nullptr);
    EXPECT_EQ(chunk.DataLast, nullptr);
    EXPECT_EQ(chunk.ChildFirst, nullptr);
    EXPECT_EQ(chunk.ChildLast, nullptr);
    EXPECT_EQ(chunk.Parent, nullptr);
    EXPECT_EQ(chunk.Next, nullptr);
    EXPECT_EQ(chunk.Prev, nullptr);
}

TEST(CHUNKTest, AddDataCopiesPayloadAndBuildsDataCache)
{
    CHUNK chunk { 1 };
    const char payload[] = "payload";

    chunk.AddData(payload, sizeof(payload));

    ASSERT_NE(chunk.DataFirst, nullptr);
    EXPECT_EQ(chunk.DataFirst, chunk.DataLast);
    EXPECT_EQ(chunk.DataFirst->Parent, &chunk);
    EXPECT_EQ(chunk.DataFirst->Size, sizeof(payload));
    EXPECT_NE(chunk.DataFirst->Data, reinterpret_cast<const uint8_t*>(payload));
    EXPECT_EQ(std::memcmp(chunk.DataFirst->Data, payload, sizeof(payload)), 0);

    auto* data = chunk.GetDataPtr(0);
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data, chunk.DataFirst);
    EXPECT_EQ(chunk.DataListCount, 1);
    ASSERT_NE(chunk.DataList, nullptr);
    EXPECT_EQ(chunk.DataList[0], data);
}

TEST(CHUNKTest, AddStringStoresNullTerminatedString)
{
    CHUNK chunk { 1 };

    chunk.AddString("hello");

    auto* data = chunk.GetDataPtr(0);
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->Size, 6);
    EXPECT_STREQ(reinterpret_cast<const char*>(data->Data), "hello");
}

TEST(CHUNKTest, RemoveDataRemovesSingleNodeAndInvalidatesCache)
{
    CHUNK chunk { 1 };
    const int first = 10;
    const int second = 20;
    const int third = 30;

    chunk.AddData(&first, sizeof(first));
    chunk.AddData(&second, sizeof(second));
    chunk.AddData(&third, sizeof(third));
    ASSERT_NE(chunk.GetDataPtr(1), nullptr);

    chunk.RemoveData(1);

    EXPECT_EQ(chunk.DataList, nullptr);
    EXPECT_EQ(chunk.DataListCount, 0);
    ASSERT_NE(chunk.DataFirst, nullptr);
    ASSERT_NE(chunk.DataLast, nullptr);
    EXPECT_EQ(*reinterpret_cast<int*>(chunk.DataFirst->Data), first);
    EXPECT_EQ(*reinterpret_cast<int*>(chunk.DataLast->Data), third);
    EXPECT_EQ(chunk.DataFirst->Next, chunk.DataLast);
    EXPECT_EQ(chunk.DataLast->Prev, chunk.DataFirst);
    EXPECT_EQ(chunk.GetDataPtr(1), chunk.DataLast);
}

TEST(CHUNKTest, RemoveDataMinusOneRemovesAllData)
{
    CHUNK chunk { 1 };
    const int first = 10;
    const int second = 20;

    chunk.AddData(&first, sizeof(first));
    chunk.AddData(&second, sizeof(second));
    ASSERT_NE(chunk.GetDataPtr(0), nullptr);

    chunk.RemoveData(-1);

    EXPECT_EQ(chunk.DataFirst, nullptr);
    EXPECT_EQ(chunk.DataLast, nullptr);
    EXPECT_EQ(chunk.DataList, nullptr);
    EXPECT_EQ(chunk.DataListCount, 0);
    EXPECT_EQ(chunk.GetDataPtr(0), nullptr);
}

TEST(CHUNKTest, NewChunkAppendsChildAndDestructorUnlinksIt)
{
    CHUNK parent { 1 };

    auto* first = parent.NewChunk(10);
    auto* second = parent.NewChunk(20);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(parent.ChildFirst, first);
    EXPECT_EQ(parent.ChildLast, second);
    EXPECT_EQ(first->Parent, &parent);
    EXPECT_EQ(second->Parent, &parent);
    EXPECT_EQ(first->Next, second);
    EXPECT_EQ(second->Prev, first);

    ZUniMemory::Delete(first);

    EXPECT_EQ(parent.ChildFirst, second);
    EXPECT_EQ(parent.ChildLast, second);
    EXPECT_EQ(second->Prev, nullptr);
    EXPECT_EQ(second->Next, nullptr);
}

TEST(CHUNKTest, SetPositionCanInsertAtRequestedIndex)
{
    CHUNK parent { 1 };
    auto* first = parent.NewChunk(10);
    auto* second = parent.NewChunk(20);
    auto* third = parent.NewChunk(30);

    third->SetPosition(&parent, 0);

    EXPECT_EQ(parent.ChildFirst, third);
    EXPECT_EQ(third->Next, first);
    EXPECT_EQ(first->Prev, third);
    EXPECT_EQ(first->Next, second);
    EXPECT_EQ(parent.ChildLast, second);
}

TEST(CHUNKTest, SetPositionNullDetachesFromParent)
{
    CHUNK parent { 1 };
    auto* child = parent.NewChunk(10);

    child->SetPosition(nullptr, -1);

    EXPECT_EQ(parent.ChildFirst, nullptr);
    EXPECT_EQ(parent.ChildLast, nullptr);
    EXPECT_EQ(child->Parent, nullptr);
    EXPECT_EQ(child->Prev, nullptr);
    EXPECT_EQ(child->Next, nullptr);

    ZUniMemory::Delete(child);
}
