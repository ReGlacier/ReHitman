#include <gtest/gtest.h>
#include <Glacier/ZSTL/CHUNK.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


using namespace Glacier;

TEST(CHUNKTest, ConstructorInitializesEmptyChunk)
{
    CHUNK chunk { 0x1234, nullptr, -1 };

    // NOTE: TotalSize and HeaderSize are intentionally NOT initialized by the
    // original constructor (PC 0x427AF0); they are only written by BuildHeaders().
    EXPECT_EQ(chunk.Name, 0x1234);
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
    CHUNK chunk { 1, nullptr, -1 };
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
    CHUNK chunk { 1, nullptr, -1 };

    chunk.AddString("hello");

    auto* data = chunk.GetDataPtr(0);
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->Size, 6);
    EXPECT_STREQ(reinterpret_cast<const char*>(data->Data), "hello");
}

TEST(CHUNKTest, RemoveDataRemovesSingleNodeAndInvalidatesCache)
{
    CHUNK chunk { 1, nullptr, -1 };
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
    CHUNK chunk { 1, nullptr, -1 };
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
    CHUNK parent { 1, nullptr, -1 };

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
    CHUNK parent { 1, nullptr, -1 };
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
    CHUNK parent { 1, nullptr, -1 };
    auto* child = parent.NewChunk(10);

    child->SetPosition(nullptr, -1);

    EXPECT_EQ(parent.ChildFirst, nullptr);
    EXPECT_EQ(parent.ChildLast, nullptr);
    EXPECT_EQ(child->Parent, nullptr);
    EXPECT_EQ(child->Prev, nullptr);
    EXPECT_EQ(child->Next, nullptr);

    ZUniMemory::Delete(child);
}

// -----------------------------------------------------------------------------
// Copy constructor (deep copy)
// -----------------------------------------------------------------------------

TEST(CHUNKTest, CopyConstructorDeepCopiesTreeAndData)
{
    CHUNK source { 7, nullptr, -1 };
    const int payload = 42;
    source.AddData(&payload, sizeof(payload));
    auto* sourceChild = source.NewChunk(8);
    const char text[] = "child";
    sourceChild->AddData(text, sizeof(text));

    auto* copy = ZUniMemory::New<CHUNK>(&source, nullptr);

    EXPECT_EQ(copy->Name, 7);
    EXPECT_EQ(copy->Parent, nullptr);
    ASSERT_EQ(copy->CountData(), 1);
    EXPECT_EQ(*reinterpret_cast<int*>(copy->GetData(0)), 42);
    ASSERT_NE(copy->ChildFirst, nullptr);
    EXPECT_EQ(copy->ChildFirst->Name, 8);
    EXPECT_EQ(copy->ChildFirst->Parent, copy);
    ASSERT_EQ(copy->ChildFirst->CountData(), 1);
    EXPECT_STREQ(reinterpret_cast<const char*>(copy->ChildFirst->GetData(0)), "child");
    // Deep copy: buffers must not alias the source
    EXPECT_NE(copy->GetData(0), source.GetData(0));
    EXPECT_NE(copy->ChildFirst, sourceChild);

    ZUniMemory::Delete(copy);
}

TEST(CHUNKTest, CopyConstructorAttachesToParentAsLastChild)
{
    CHUNK source { 7, nullptr, -1 };
    CHUNK parent { 1, nullptr, -1 };
    auto* existing = parent.NewChunk(2);

    auto* copy = ZUniMemory::New<CHUNK>(&source, &parent);

    EXPECT_EQ(copy->Parent, &parent);
    EXPECT_EQ(copy->Prev, existing);
    EXPECT_EQ(existing->Next, copy);
    EXPECT_EQ(parent.ChildLast, copy);
    EXPECT_EQ(parent.ChildFirst, existing);

    ZUniMemory::Delete(copy);
}

// -----------------------------------------------------------------------------
// InsertData / ReplaceData
// -----------------------------------------------------------------------------

TEST(CHUNKTest, InsertDataInsertsBeforeGivenIndex)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int first = 10, second = 20, inserted = 15;

    chunk.AddData(&first, sizeof(first));
    chunk.AddData(&second, sizeof(second));
    chunk.InsertData(1, const_cast<int*>(&inserted), sizeof(inserted));

    ASSERT_EQ(chunk.CountData(), 3);
    EXPECT_EQ(*reinterpret_cast<int*>(chunk.GetData(0)), 10);
    EXPECT_EQ(*reinterpret_cast<int*>(chunk.GetData(1)), 15);
    EXPECT_EQ(*reinterpret_cast<int*>(chunk.GetData(2)), 20);
}

TEST(CHUNKTest, InsertDataWithInvalidIndexDoesNothing)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int first = 10;

    chunk.AddData(&first, sizeof(first));
    chunk.InsertData(5, const_cast<int*>(&first), sizeof(first));

    EXPECT_EQ(chunk.CountData(), 1);
}

TEST(CHUNKTest, ReplaceDataOverwritesExistingPayload)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int first = 10;
    const char replacement[] = "replacement-data";

    chunk.AddData(&first, sizeof(first));
    chunk.ReplaceData(0, replacement, sizeof(replacement));

    ASSERT_EQ(chunk.CountData(), 1);
    EXPECT_EQ(chunk.GetDataSize(0), sizeof(replacement));
    EXPECT_EQ(std::memcmp(chunk.GetData(0), replacement, sizeof(replacement)), 0);
}

TEST(CHUNKTest, ReplaceDataWithInvalidIndexDoesNothing)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int first = 10;
    const int other = 99;

    chunk.AddData(&first, sizeof(first));
    chunk.ReplaceData(3, &other, sizeof(other));

    ASSERT_EQ(chunk.CountData(), 1);
    EXPECT_EQ(*reinterpret_cast<int*>(chunk.GetData(0)), 10);
}

// -----------------------------------------------------------------------------
// CopyData
// -----------------------------------------------------------------------------

TEST(CHUNKTest, CopyDataAppendsFromStartIndex)
{
    CHUNK source { 1, nullptr, -1 };
    CHUNK target { 2, nullptr, -1 };
    const int values[4] = { 1, 2, 3, 4 };

    for (const int value : values)
    {
        source.AddData(&value, sizeof(value));
    }

    source.CopyData(&target, 2);

    ASSERT_EQ(target.CountData(), 2);
    EXPECT_EQ(*reinterpret_cast<int*>(target.GetData(0)), 3);
    EXPECT_EQ(*reinterpret_cast<int*>(target.GetData(1)), 4);
}

TEST(CHUNKTest, CopyDataWithZeroIndexCopiesEverything)
{
    CHUNK source { 1, nullptr, -1 };
    CHUNK target { 2, nullptr, -1 };
    const int values[2] = { 1, 2 };

    for (const int value : values)
    {
        source.AddData(&value, sizeof(value));
    }

    source.CopyData(&target, 0);

    EXPECT_EQ(target.CountData(), 2);
}

// -----------------------------------------------------------------------------
// Count / Find / Get helpers
// -----------------------------------------------------------------------------

TEST(CHUNKTest, CountChildrenAndCountData)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int payload = 5;

    EXPECT_EQ(chunk.CountChildren(), 0);
    EXPECT_EQ(chunk.CountData(), 0);

    chunk.NewChunk(10);
    chunk.NewChunk(20);
    chunk.AddData(&payload, sizeof(payload));

    EXPECT_EQ(chunk.CountChildren(), 2);
    EXPECT_EQ(chunk.CountData(), 1);
}

TEST(CHUNKTest, FindChildByNameReturnsMatchingChild)
{
    CHUNK parent { 1, nullptr, -1 };
    auto* first = parent.NewChunk(10);
    auto* second = parent.NewChunk(20);

    EXPECT_EQ(parent.FindChild(20), second);
    EXPECT_EQ(parent.FindChild(10), first);
    EXPECT_EQ(parent.FindChild(30), nullptr);
}

TEST(CHUNKTest, FindChildByPointerReturnsIndex)
{
    CHUNK parent { 1, nullptr, -1 };
    auto* first = parent.NewChunk(10);
    auto* second = parent.NewChunk(20);
    CHUNK outsider { 30, nullptr, -1 };

    EXPECT_EQ(parent.FindChild(first), 0);
    EXPECT_EQ(parent.FindChild(second), 1);
    EXPECT_EQ(parent.FindChild(&outsider), -1);
}

TEST(CHUNKTest, GetChildUsesOneBasedIndexing)
{
    CHUNK parent { 1, nullptr, -1 };
    auto* first = parent.NewChunk(10);
    auto* second = parent.NewChunk(20);

    EXPECT_EQ(parent.GetChild(1), first);
    EXPECT_EQ(parent.GetChild(2), second);
    EXPECT_EQ(parent.GetChild(3), nullptr);
    EXPECT_EQ(parent.GetChild(0), nullptr);
}

TEST(CHUNKTest, TypedGettersReturnInterpretedPayload)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int longValue = 0x11223344;
    const float floatValue = 1.5f;
    const double doubleValue = 2.5;
    const char charValue = 'x';

    chunk.AddData(&longValue, sizeof(longValue));
    chunk.AddData(&floatValue, sizeof(floatValue));
    chunk.AddData(&doubleValue, sizeof(doubleValue));
    chunk.AddString("str");
    chunk.AddData(&charValue, sizeof(charValue));

    EXPECT_EQ(chunk.GetLong(0), longValue);
    EXPECT_FLOAT_EQ(chunk.GetFloat(1), floatValue);
    EXPECT_DOUBLE_EQ(chunk.GetDouble(2), doubleValue);
    EXPECT_STREQ(chunk.GetString(3), "str");
    EXPECT_EQ(chunk.GetChar(4), charValue);
}

TEST(CHUNKTest, GetDataOutOfRangeReturnsNull)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int payload = 5;

    chunk.AddData(&payload, sizeof(payload));

    EXPECT_EQ(chunk.GetData(1), nullptr);
    EXPECT_EQ(chunk.GetDataSize(1), 0);
}

// -----------------------------------------------------------------------------
// AddChunk (virtual alias for NewChunk)
// -----------------------------------------------------------------------------

TEST(CHUNKTest, AddChunkAppendsChildLikeNewChunk)
{
    CHUNK parent { 1, nullptr, -1 };

    auto* child = parent.AddChunk(55);

    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->Name, 55);
    EXPECT_EQ(child->Parent, &parent);
    EXPECT_EQ(parent.ChildFirst, child);
    EXPECT_EQ(parent.ChildLast, child);
}

// -----------------------------------------------------------------------------
// BaseToDerived / DerivedToBase
// -----------------------------------------------------------------------------

TEST(CHUNKTest, DerivedToBaseCopiesDataToTarget)
{
    CHUNK source { 1, nullptr, -1 };
    CHUNK target { 2, nullptr, -1 };
    const int values[2] = { 7, 8 };

    for (const int value : values)
    {
        source.AddData(&value, sizeof(value));
    }

    source.DerivedToBase(&target);

    ASSERT_EQ(target.CountData(), 2);
    EXPECT_EQ(*reinterpret_cast<int*>(target.GetData(0)), 7);
    EXPECT_EQ(*reinterpret_cast<int*>(target.GetData(1)), 8);
}

TEST(CHUNKTest, BaseToDerivedCopiesChildrenAndData)
{
    CHUNK base { 1, nullptr, -1 };
    const int payload = 11;
    base.AddData(&payload, sizeof(payload));
    base.NewChunk(3);

    CHUNK derived { 2, nullptr, -1 };
    derived.BaseToDerived(&base);

    EXPECT_EQ(derived.CountChildren(), 1);
    ASSERT_EQ(derived.CountData(), 1);
    EXPECT_EQ(*reinterpret_cast<int*>(derived.GetData(0)), 11);
}

// -----------------------------------------------------------------------------
// BuildHeaders / GetFileLen / SaveToMem (binary format)
// -----------------------------------------------------------------------------

TEST(CHUNKTest, BuildHeadersSingleDataChunkLayout)
{
    CHUNK chunk { 0xABCD, nullptr, -1 };
    const int payload = 0x11223344; // 4 bytes

    chunk.AddData(&payload, sizeof(payload));
    const int totalSize = chunk.BuildHeaders(false);

    // Single data block, no children -> no flags, header = [Name, TotalSize]
    // Return value includes the header size: data(4) + header(8) = 12.
    EXPECT_EQ(chunk.HeaderSize, 8);
    EXPECT_EQ(totalSize, 12);
    ASSERT_NE(chunk.Header, nullptr);
    EXPECT_EQ(chunk.Header[0], 0xABCDu);
    // TotalSize = data(4) + header(8)
    EXPECT_EQ(chunk.Header[1] & 0x3FFFFFFFu, 12u);
    EXPECT_EQ(chunk.Header[1] & 0xC0000000u, 0u);
}

TEST(CHUNKTest, BuildHeadersAlignsDataSizeToFourBytes)
{
    CHUNK chunk { 1, nullptr, -1 };
    const char payload[3] = { 1, 2, 3 }; // 3 bytes -> aligned to 4

    chunk.AddData(payload, sizeof(payload));
    const int totalSize = chunk.BuildHeaders(false);

    EXPECT_EQ(totalSize, 12); // 4 (data, aligned) + 8 (header)
    EXPECT_EQ(chunk.Header[1] & 0x3FFFFFFFu, 12u);
}

TEST(CHUNKTest, BuildHeadersMultipleDataSetsExtendedFlag)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int first = 1, second = 2;

    chunk.AddData(&first, sizeof(first));
    chunk.AddData(&second, sizeof(second));
    chunk.BuildHeaders(false);

    // bit 30 set -> extended header with data table:
    // [Name, TotalSize, DataOffset, DataCount, Size0, Size1]
    EXPECT_EQ(chunk.Header[1] & 0x40000000u, 0x40000000u);
    EXPECT_EQ(chunk.Header[1] & 0x80000000u, 0u);
    EXPECT_EQ(chunk.HeaderSize, 24);
    EXPECT_EQ(chunk.Header[3], 2u); // data count
    EXPECT_EQ(chunk.Header[4], 4u); // size of block 0
    EXPECT_EQ(chunk.Header[5], 4u); // size of block 1
}

TEST(CHUNKTest, BuildHeadersCompactSuppressesDataTable)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int first = 1, second = 2;

    chunk.AddData(&first, sizeof(first));
    chunk.AddData(&second, sizeof(second));
    chunk.BuildHeaders(true);

    // Compact mode: data count collapsed to 1, no table, no bit 30
    EXPECT_EQ(chunk.Header[1] & 0xC0000000u, 0u);
    EXPECT_EQ(chunk.HeaderSize, 8);
}

TEST(CHUNKTest, BuildHeadersWithChildrenSetsChildFlag)
{
    CHUNK parent { 1, nullptr, -1 };
    auto* child = parent.NewChunk(2);
    const int payload = 9;

    child->AddData(&payload, sizeof(payload));
    parent.BuildHeaders(false);

    // Parent: bit 31 set (has children)
    EXPECT_EQ(parent.Header[1] & 0x80000000u, 0x80000000u);
    // [Name, TotalSize, DataOffset, ChildCount]
    EXPECT_EQ(parent.HeaderSize, 16);
    EXPECT_EQ(parent.Header[3], 1u); // one child
}

TEST(CHUNKTest, GetFileLenReturnsTotalSizeWithoutFlags)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int payload = 5;

    chunk.AddData(&payload, sizeof(payload));

    EXPECT_EQ(chunk.GetFileLen(), 12); // 4 data + 8 header
}

TEST(CHUNKTest, SaveToMemSerializesHeaderAndData)
{
    CHUNK chunk { 0x1234, nullptr, -1 };
    const int payload = 0x0A0B0C0D;

    chunk.AddData(&payload, sizeof(payload));

    alignas(16) char buffer[64] = {};
    chunk.SaveToMem(buffer, false);

    const auto* words = reinterpret_cast<const uint32_t*>(buffer);
    EXPECT_EQ(words[0], 0x1234u);        // Name
    EXPECT_EQ(words[1] & 0x3FFFFFFFu, 12u); // TotalSize
    EXPECT_EQ(words[2], 0x0A0B0C0Du);    // payload follows the 8-byte header
}

TEST(CHUNKTest, SaveToMemSerializesChildAfterParentHeader)
{
    CHUNK parent { 1, nullptr, -1 };
    auto* child = parent.NewChunk(2);
    const int payload = 0x77;

    child->AddData(&payload, sizeof(payload));

    alignas(16) char buffer[128] = {};
    parent.SaveToMem(buffer, false);

    const auto* words = reinterpret_cast<const uint32_t*>(buffer);
    // Parent header: [Name, TotalSize, DataOffset, ChildCount]
    EXPECT_EQ(words[0], 1u);
    EXPECT_EQ(words[1] & 0x80000000u, 0x80000000u);
    EXPECT_EQ(words[3], 1u);
    // Child header follows at offset 16: [Name, TotalSize]
    EXPECT_EQ(words[4], 2u);
    EXPECT_EQ(words[5] & 0x3FFFFFFFu, 12u);
    // Child payload at offset 24
    EXPECT_EQ(words[6], 0x77u);
}

// -----------------------------------------------------------------------------
// Print (smoke test: must not crash with and without data/children)
// -----------------------------------------------------------------------------

TEST(CHUNKTest, PrintDoesNotCrash)
{
    CHUNK chunk { 1, nullptr, -1 };
    const int payload = 5;

    chunk.AddData(&payload, sizeof(payload));
    chunk.NewChunk(2);

    chunk.Print(0); // builds headers first
    chunk.Print(6);
}
