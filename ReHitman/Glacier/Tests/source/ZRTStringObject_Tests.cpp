#include <Glacier/ZSTL/ZRTStringObject.h>
#include <gtest/gtest.h>

#include <cstring>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZStringMemoryManager::ZHeader::ZRefAndSize) == 0x4);
    static_assert(sizeof(ZStringMemoryManager::ZHeader) == 0x4);
    static_assert(sizeof(ZStringMemoryManager::ZFreeFooter) == 0x4);
    static_assert(sizeof(ZRTStringObject) == 0xC);
    static_assert(sizeof(ZRTString) == 0x4);

    uint32_t ExpectedHashCode(const char* pString)
    {
        int16_t hash = 0;
        while (*pString)
        {
            hash = static_cast<int16_t>(8 * hash + *pString);
            ++pString;
        }

        return static_cast<uint16_t>(hash) & 0x0FFF;
    }
}

TEST(ZRTStringObject, RefAndSizeDefaultConstructsAsEndMarker)
{
    ZStringMemoryManager::ZHeader::ZRefAndSize refAndSize;

    EXPECT_TRUE(refAndSize.IsEnd());
    EXPECT_FALSE(refAndSize.IsFree());
    EXPECT_EQ(refAndSize.m_Data, 0x7FFF0000u);
}

TEST(ZRTStringObject, RefAndSizeStoresFreeSize)
{
    ZStringMemoryManager::ZHeader::ZRefAndSize refAndSize(true, 0x12345);

    EXPECT_TRUE(refAndSize.IsFree());
    EXPECT_FALSE(refAndSize.IsEnd());
    EXPECT_EQ(refAndSize.GetSize(), 0x12345u);
}

TEST(ZRTStringObject, RefAndSizeStoresAllocatedSizeAndRefCounter)
{
    ZStringMemoryManager::ZHeader::ZRefAndSize refAndSize(false, 64);

    EXPECT_FALSE(refAndSize.IsFree());
    EXPECT_EQ(refAndSize.GetSize(), 64u);

    refAndSize.SetRefCounter(7);

    EXPECT_EQ(refAndSize.GetRefCounter(), 7u);
    EXPECT_EQ(refAndSize.GetSize(), 64u);
}

TEST(ZRTStringObject, RefCounterOnFreeBlockAsserts)
{
    ZStringMemoryManager::ZHeader::ZRefAndSize refAndSize(true, 64);

    EXPECT_THROW(refAndSize.GetRefCounter(), std::runtime_error);
    EXPECT_THROW(refAndSize.SetRefCounter(1), std::runtime_error);
}

TEST(ZRTStringObject, FreeFooterFindsOwningHeader)
{
    alignas(ZStringMemoryManager::ZFreeHeader) unsigned char storage[64]{};
    auto* header = new (storage) ZStringMemoryManager::ZFreeHeader(sizeof(storage));

    EXPECT_EQ(header->GetFooter()->GetSize(), sizeof(storage));
    EXPECT_EQ(header->GetFooter()->GetHeader(), header);
}

TEST(ZRTStringObject, ManagerInternsSameStringObject)
{
    ZStringMemoryManager manager(1024);

    ZRTStringObject* first = manager.GetStringObject("alpha");
    ZRTStringObject* second = manager.GetStringObject("alpha");
    ZRTStringObject* third = manager.GetStringObject("beta");

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second);
    EXPECT_NE(first, third);
    EXPECT_STREQ(first->c_str(), "alpha");
    EXPECT_STREQ(third->c_str(), "beta");
    EXPECT_TRUE(manager.Check());
}

TEST(ZRTStringObject, ZRTStringConstructAssignCompareAndDereference)
{
    ZRTString empty;
    EXPECT_EQ(empty.c_str(), nullptr);
    EXPECT_EQ(empty.Compare(nullptr), 0);
    EXPECT_LT(empty.Compare("abc"), 0);

    ZRTString string("bravo");
    EXPECT_STREQ(string.c_str(), "bravo");
    EXPECT_TRUE(string == "bravo");
    EXPECT_TRUE(string < "charlie");
    EXPECT_EQ(*string, 'b');

    string = "alpha";
    EXPECT_STREQ(string.c_str(), "alpha");
    EXPECT_TRUE(string < "bravo");
}

TEST(ZRTStringObject, ZRTStringCopySharesInternedObjectAndTracksRefs)
{
    ZRTString original("shared");
    ASSERT_NE(original.m_StringObject, nullptr);
    EXPECT_EQ(original.m_StringObject->m_RefOrSize.GetRefCounter(), 1u);

    {
        ZRTString copy(original);
        EXPECT_EQ(copy.m_StringObject, original.m_StringObject);
        EXPECT_EQ(original.m_StringObject->m_RefOrSize.GetRefCounter(), 2u);

        ZRTString assigned;
        assigned = original;
        EXPECT_EQ(assigned.m_StringObject, original.m_StringObject);
        EXPECT_EQ(original.m_StringObject->m_RefOrSize.GetRefCounter(), 3u);
    }

    EXPECT_EQ(original.m_StringObject->m_RefOrSize.GetRefCounter(), 1u);
}

TEST(ZRTStringObject, ManagerReusesFreedStringBlock)
{
    ZRTStringObject* firstObject = nullptr;

    {
        ZRTString first("reuse-me");
        firstObject = first.m_StringObject;
        ASSERT_NE(firstObject, nullptr);
    }

    ZRTString second("reuse-me");

    EXPECT_EQ(second.m_StringObject, firstObject);
    EXPECT_STREQ(second.c_str(), "reuse-me");
}

TEST(ZRTStringObject, HeaderWrapsRefAndSizeQueries)
{
    ZStringMemoryManager::ZHeader endHeader;
    EXPECT_TRUE(endHeader.IsEnd());
    EXPECT_FALSE(endHeader.IsFreeHeader());

    ZStringMemoryManager::ZHeader freeHeader(true, 128);
    EXPECT_TRUE(freeHeader.IsFreeHeader());
    EXPECT_FALSE(freeHeader.IsEnd());
    EXPECT_EQ(freeHeader.GetSize(), 128u);

    ZStringMemoryManager::ZHeader allocatedHeader(false, 24);
    EXPECT_FALSE(allocatedHeader.IsFreeHeader());
    EXPECT_EQ(allocatedHeader.GetSize(), 24u);
}

TEST(ZRTStringObject, SetAllocatedSizeRejectsValuesOutsideSixteenBits)
{
    ZStringMemoryManager::ZHeader::ZRefAndSize refAndSize;

    EXPECT_THROW(refAndSize.SetAllocatedSize(0x10000), std::runtime_error);
}

TEST(ZRTStringObject, SetFreeSizeRejectsValuesUsingFreeFlagBit)
{
    ZStringMemoryManager::ZHeader::ZRefAndSize refAndSize;

    EXPECT_THROW(refAndSize.SetFreeSize(0x80000000), std::runtime_error);
}

TEST(ZRTStringObject, ResizeRequiresFreeBlock)
{
    ZStringMemoryManager::ZHeader::ZRefAndSize refAndSize(false, 64);

    EXPECT_THROW(refAndSize.Resize(32), std::runtime_error);
}

TEST(ZRTStringObject, FreeHeaderResizeSplitsFromTailAndLeavesValidFooter)
{
    alignas(ZStringMemoryManager::ZFreeHeader) unsigned char storage[96]{};
    auto* freeHeader = new (storage) ZStringMemoryManager::ZFreeHeader(sizeof(storage));

    ZStringMemoryManager::ZHeader* allocatedHeader = freeHeader->Resize(24);

    ASSERT_NE(allocatedHeader, nullptr);
    EXPECT_EQ(freeHeader->GetSize(), 72u);
    EXPECT_EQ(allocatedHeader, reinterpret_cast<ZStringMemoryManager::ZHeader*>(storage + 72));
    EXPECT_EQ(freeHeader->GetFooter()->GetSize(), 72u);
    EXPECT_EQ(freeHeader->GetFooter()->GetHeader(), freeHeader);
}

TEST(ZRTStringObject, FreeHeaderResizeReturnsNullWhenRemainderWouldBeTooSmall)
{
    alignas(ZStringMemoryManager::ZFreeHeader) unsigned char storage[32]{};
    auto* freeHeader = new (storage) ZStringMemoryManager::ZFreeHeader(sizeof(storage));

    EXPECT_EQ(freeHeader->Resize(24), nullptr);
    EXPECT_EQ(freeHeader->GetSize(), sizeof(storage));
    EXPECT_EQ(freeHeader->GetFooter()->GetHeader(), freeHeader);
}

TEST(ZRTStringObject, FreeHeaderExtendMergesSizeAndMovesFooter)
{
    alignas(ZStringMemoryManager::ZFreeHeader) unsigned char storage[128]{};
    auto* freeHeader = new (storage) ZStringMemoryManager::ZFreeHeader(64);

    freeHeader->Extend(32);

    EXPECT_EQ(freeHeader->GetSize(), 96u);
    EXPECT_EQ(freeHeader->GetFooter(), reinterpret_cast<ZStringMemoryManager::ZFreeFooter*>(storage + 92));
    EXPECT_EQ(freeHeader->GetFooter()->GetHeader(), freeHeader);
}

TEST(ZRTStringObject, HashCodeMatchesOriginalSignedShortAlgorithm)
{
    ZStringMemoryManager manager(256);

    EXPECT_EQ(manager.GetHashCode(""), 0u);
    EXPECT_EQ(manager.GetHashCode("a"), ExpectedHashCode("a"));
    EXPECT_EQ(manager.GetHashCode("alpha"), ExpectedHashCode("alpha"));
    EXPECT_EQ(manager.GetHashCode("a much longer serializer field name"), ExpectedHashCode("a much longer serializer field name"));
    EXPECT_LT(manager.GetHashCode("anything"), 4096u);
}

TEST(ZRTStringObject, NullStringObjectLookupReturnsNull)
{
    ZStringMemoryManager manager(256);

    EXPECT_EQ(manager.GetStringObject(nullptr), nullptr);
    EXPECT_TRUE(manager.Check());
}

TEST(ZRTStringObject, AllocRoundsObjectSizeToFourAndMinimumSixteenBytes)
{
    ZStringMemoryManager manager(128);

    ZRTStringObject* tiny = manager.Alloc("");
    ASSERT_NE(tiny, nullptr);
    EXPECT_EQ(tiny->GetSize(), 16u);
    EXPECT_STREQ(tiny->c_str(), "");
    EXPECT_EQ(reinterpret_cast<uintptr_t>(tiny) & 3u, 0u);

    ZRTStringObject* larger = manager.Alloc("12345");
    ASSERT_NE(larger, nullptr);
    EXPECT_EQ(larger->GetSize(), 20u);
    EXPECT_STREQ(larger->c_str(), "12345");
    EXPECT_EQ(reinterpret_cast<uintptr_t>(larger) & 3u, 0u);
}

TEST(ZRTStringObject, AllocWritesTerminatorAtEndOfAllocatedBlock)
{
    ZStringMemoryManager manager(128);

    ZRTStringObject* object = manager.Alloc("abc");
    ASSERT_NE(object, nullptr);

    const char* raw = reinterpret_cast<const char*>(object);
    EXPECT_EQ(raw[object->GetSize() - 1], '\0');
    EXPECT_STREQ(object->c_str(), "abc");
}

TEST(ZRTStringObject, AllocReturnsNullWhenPoolIsExhausted)
{
    ZStringMemoryManager manager(32);

    ASSERT_NE(manager.Alloc("first"), nullptr);
    EXPECT_EQ(manager.Alloc("second"), nullptr);
    EXPECT_TRUE(manager.Check());
}

TEST(ZRTStringObject, FreeCoalescesWithNextFreeBlock)
{
    ZStringMemoryManager manager(128);
    ZRTStringObject* first = manager.Alloc("first");
    ZRTStringObject* second = manager.Alloc("second");
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    manager.Free(second);
    manager.Free(first);

    EXPECT_EQ(manager.m_FreeList.Count(), 1);
    ASSERT_NE(manager.m_FreeList.GetFirst(), nullptr);
    auto* freeHeader = static_cast<ZStringMemoryManager::ZFreeHeader*>(static_cast<ZListNode<ZStringMemoryManager::ZFreeHeader, 0>*>(manager.m_FreeList.GetFirst()));
    EXPECT_EQ(freeHeader, manager.m_StringObjectStart);
    EXPECT_EQ(freeHeader->GetSize(), 128u);
    EXPECT_TRUE(manager.Check());
}

TEST(ZRTStringObject, FreeCoalescesWithPreviousFreeBlock)
{
    ZStringMemoryManager manager(128);
    ZRTStringObject* first = manager.Alloc("first");
    ZRTStringObject* second = manager.Alloc("second");
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    manager.Free(first);
    manager.Free(second);

    EXPECT_EQ(manager.m_FreeList.Count(), 1);
    ASSERT_NE(manager.m_FreeList.GetFirst(), nullptr);
    auto* freeHeader = static_cast<ZStringMemoryManager::ZFreeHeader*>(static_cast<ZListNode<ZStringMemoryManager::ZFreeHeader, 0>*>(manager.m_FreeList.GetFirst()));
    EXPECT_EQ(freeHeader, manager.m_StringObjectStart);
    EXPECT_EQ(freeHeader->GetSize(), 128u);
    EXPECT_TRUE(manager.Check());
}

TEST(ZRTStringObject, FreeRejectsObjectsOutsideManagerBuffer)
{
    ZStringMemoryManager manager(128);
    ZStringMemoryManager otherManager(128);
    ZRTStringObject* foreignObject = otherManager.Alloc("foreign");
    ASSERT_NE(foreignObject, nullptr);

    EXPECT_THROW(manager.Free(foreignObject), std::runtime_error);
}

TEST(ZRTStringObject, ZRTStringNullAssignmentCleansUpToEmpty)
{
    ZRTString string("temporary");
    ASSERT_NE(string.m_StringObject, nullptr);

    string = static_cast<const char*>(nullptr);

    EXPECT_EQ(string.m_StringObject, nullptr);
    EXPECT_EQ(string.c_str(), nullptr);
    EXPECT_EQ(string.Compare(nullptr), 0);
}

TEST(ZRTStringObject, ZRTStringSelfAssignmentKeepsObjectAndRefCount)
{
    ZRTString string("self");
    ZRTStringObject* object = string.m_StringObject;
    ASSERT_NE(object, nullptr);
    ASSERT_EQ(object->m_RefOrSize.GetRefCounter(), 1u);

    string = string;

    EXPECT_EQ(string.m_StringObject, object);
    EXPECT_EQ(object->m_RefOrSize.GetRefCounter(), 1u);
}

TEST(ZRTStringObject, ZRTStringAssignmentReleasesPreviousObject)
{
    ZRTString owner("old-value");
    ZRTString string(owner);
    ZRTStringObject* oldObject = string.m_StringObject;
    ASSERT_NE(oldObject, nullptr);
    ASSERT_EQ(oldObject->m_RefOrSize.GetRefCounter(), 2u);

    string = "new-value";

    ASSERT_NE(string.m_StringObject, nullptr);
    EXPECT_EQ(owner.m_StringObject, oldObject);
    EXPECT_EQ(oldObject->m_RefOrSize.GetRefCounter(), 1u);
    EXPECT_STREQ(string.c_str(), "new-value");
}

TEST(ZRTStringObject, ZRTStringDereferenceNullAsserts)
{
    ZRTString string;

    EXPECT_THROW(*string, std::runtime_error);
}

TEST(ZRTStringObject, ZRTStringCompareHandlesNullAndLexicographicOrdering)
{
    ZRTString alpha("alpha");

    EXPECT_GT(alpha.Compare(nullptr), 0);
    EXPECT_EQ(alpha.Compare("alpha"), 0);
    EXPECT_LT(alpha.Compare("beta"), 0);
    EXPECT_GT(alpha.Compare("aardvark"), 0);
    EXPECT_FALSE(alpha < nullptr);
}

TEST(ZRTStringObject, ManualRefCountRoundTripAndUnderflowAssert)
{
    ZStringMemoryManager manager(128);
    ZRTStringObject* object = manager.Alloc("manual");
    ASSERT_NE(object, nullptr);

    EXPECT_EQ(object->m_RefOrSize.GetRefCounter(), 0u);
    EXPECT_EQ(object->IncRef(), 1u);
    EXPECT_TRUE(object->DecRef());
    EXPECT_THROW(object->DecRef(), std::runtime_error);

    manager.Free(object);
}

TEST(ZRTStringObject, ManyInternedStringsRemainSearchableAcrossBuckets)
{
    ZStringMemoryManager manager(4096);
    const char* strings[] = {
        "one", "two", "three", "four", "five", "six", "seven", "eight",
        "nine", "ten", "serializer", "dictionary", "packed", "token", "hash", "bucket"
    };

    for (const char* pString : strings)
    {
        ZRTStringObject* object = manager.GetStringObject(pString);
        ASSERT_NE(object, nullptr);
        EXPECT_STREQ(object->c_str(), pString);
    }

    for (const char* pString : strings)
    {
        ZRTStringObject* first = manager.GetStringObject(pString);
        ZRTStringObject* second = manager.GetStringObject(pString);
        EXPECT_EQ(first, second);
    }

    EXPECT_TRUE(manager.Check());
}

TEST(ZRTStringObject, ManagerNukeItResetsSingletonStorage)
{
    ZStringMemoryManager* manager = ZStringMemoryManager::Instance();
    ZRTStringObject* before = manager->GetStringObject("after-nuke");
    ASSERT_NE(before, nullptr);

    manager->NukeIt();

    ZStringMemoryManager* resetManager = ZStringMemoryManager::Instance();
    ZRTStringObject* after = resetManager->GetStringObject("after-nuke");
    ASSERT_NE(after, nullptr);
    EXPECT_STREQ(after->c_str(), "after-nuke");
    EXPECT_TRUE(resetManager->Check());
}
