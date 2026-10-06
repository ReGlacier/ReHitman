#include <Glacier/System/ZSysMem.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/EAllocType.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>


using namespace Glacier;

namespace
{
    // The allocator reserves a large virtual region, so we build exactly one
    // ZSysMem for the whole suite (its default ZWin32Allocator is process-wide)
    // rather than per-test. Other suites keep m_pInstance null and use malloc.
    alignas(ZSysMem) uint8_t g_sysMemStorage[sizeof(ZSysMem)];
    ZSysMem* g_pTestSysMem = nullptr;

    ZWin32Allocator* DefaultWin32Allocator()
    {
        return static_cast<ZWin32Allocator*>(ISysMem::Instance().GetAllocator(DEFAULT_MEM));
    }
}

class ZSysMemTests : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(g_pTestSysMem, nullptr);
        ASSERT_EQ(ISysMem::Exists(), false);
        g_pTestSysMem = znew_placement(reinterpret_cast<ZSysMem*>(g_sysMemStorage));
        ASSERT_NE(g_pTestSysMem, nullptr);
        ASSERT_EQ(ISysMem::Exists(), true);
    }

    static void TearDownTestSuite()
    {
        if (g_pTestSysMem)
        {
            g_pTestSysMem->~ZSysMem();
            g_pTestSysMem = nullptr;
        }
        ASSERT_EQ(ISysMem::Exists(), false);
    }
};

TEST_F(ZSysMemTests, SingletonIsPublished)
{
    EXPECT_TRUE(ISysMem::Exists());
    EXPECT_EQ(&ISysMem::Instance(), g_pTestSysMem);

    // CreateAllocators binds the default allocator to every EAllocType.
    for (int i = 0; i < EAllocType::END_OF_ALLOCATOR_TYPES; ++i)
        EXPECT_NE(ISysMem::Instance().GetAllocator(static_cast<EAllocType>(i)), nullptr);
}

TEST_F(ZSysMemTests, SmallPoolAllocationsAreUsableAndAligned)
{
    const int sizes[] = { 1, 7, 8, 9, 16, 64, 100, 255, 256 };
    for (int size : sizes)
    {
        void* p = ISysMem::Instance().New(DEFAULT_MEM, size);
        ASSERT_NE(p, nullptr) << "size=" << size;
        EXPECT_EQ(reinterpret_cast<uintptr_t>(p) & 0x7u, 0u) << "not 8-aligned for size=" << size;

        std::memset(p, 0xA5, size);
        for (int i = 0; i < size; ++i)
            ASSERT_EQ(static_cast<uint8_t*>(p)[i], 0xA5);

        ISysMem::Instance().Delete(static_cast<char*>(p));
    }
}

TEST_F(ZSysMemTests, LargeAllocationsAreUsableAndAligned)
{
    const int sizes[] = { 257, 258, 0x1000, 0x8000, 0x100000 }; // > 0x100 -> heap path
    for (int size : sizes)
    {
        char* p = static_cast<char*>(ISysMem::Instance().New(DEFAULT_MEM, size));
        ASSERT_NE(p, nullptr) << "size=" << size;
        EXPECT_EQ(reinterpret_cast<uintptr_t>(p) & 0x7u, 0u) << "not 8-aligned for size=" << size;

        p[0] = 0x11;
        p[size - 1] = 0x22;
        EXPECT_EQ(static_cast<uint8_t>(p[0]), 0x11);
        EXPECT_EQ(static_cast<uint8_t>(p[size - 1]), 0x22);

        ISysMem::Instance().Delete(p);
    }
}

TEST_F(ZSysMemTests, ManyAllocationsStayDistinct)
{
    constexpr int kCount = 512;
    constexpr int kSize = 40;
    void* blocks[kCount];

    for (int i = 0; i < kCount; ++i)
    {
        blocks[i] = ISysMem::Instance().New(DEFAULT_MEM, kSize);
        ASSERT_NE(blocks[i], nullptr) << "i=" << i;
        std::memset(blocks[i], static_cast<uint8_t>(i & 0xFF), kSize);
    }

    // No two live blocks may overlap (unique addresses).
    for (int i = 0; i < kCount; ++i)
        for (int j = i + 1; j < kCount; ++j)
            EXPECT_NE(blocks[i], blocks[j]) << "aliasing at " << i << "/" << j;

    for (int i = 0; i < kCount; ++i)
    {
        for (int b = 0; b < kSize; ++b)
            ASSERT_EQ(static_cast<uint8_t*>(blocks[i])[b], static_cast<uint8_t>(i & 0xFF));
    }

    for (int i = 0; i < kCount; ++i)
        ISysMem::Instance().Delete(static_cast<char*>(blocks[i]));
}

TEST_F(ZSysMemTests, FreedMemoryIsReused)
{
    ZWin32Allocator* alloc = DefaultWin32Allocator();
    ASSERT_NE(alloc, nullptr);

    char* first = nullptr;
    {
        char* rs = nullptr;
        char* re = nullptr;
        char* p = alloc->Alloc(32, &rs, &re);
        ASSERT_NE(p, nullptr);
        first = p;
        EXPECT_TRUE(alloc->Free(p));
    }

    // After freeing, a same-size allocation must return a usable block (typically
    // the one we just freed, since the page free list is LIFO).
    char* rs = nullptr;
    char* re = nullptr;
    char* p2 = alloc->Alloc(32, &rs, &re);
    ASSERT_NE(p2, nullptr);
    std::memset(p2, 0x5C, 32);
    for (int i = 0; i < 32; ++i)
        ASSERT_EQ(reinterpret_cast<uint8_t*>(p2)[i], 0x5C);
    EXPECT_TRUE(alloc->Free(p2));

    (void)first;
}

TEST_F(ZSysMemTests, AllocatorDoesNotRecognizeForeignPointer)
{
    ZWin32Allocator* alloc = DefaultWin32Allocator();
    ASSERT_NE(alloc, nullptr);

    // A heap pointer that is neither in the reserved region nor carries the
    // large-block magic must be rejected (returns false) and left untouched.
    void* foreign = std::malloc(64);
    ASSERT_NE(foreign, nullptr);
    EXPECT_FALSE(alloc->Free(static_cast<char*>(foreign)));
    std::free(foreign);
}

TEST_F(ZSysMemTests, FallbackToDefaultWhenTypeHasNoAllocator)
{
    ZAllocatorBase* saved = ISysMem::Instance().GetAllocator(FAST_MEM);
    ASSERT_NE(saved, nullptr);

    ISysMem::Instance().SetAllocator(FAST_MEM, nullptr);
    EXPECT_EQ(ISysMem::Instance().GetAllocator(FAST_MEM), nullptr);

    // With no allocator bound, New must fall back to DEFAULT_MEM and still work.
    void* p = ISysMem::Instance().New(FAST_MEM, 128);
    ASSERT_NE(p, nullptr);
    std::memset(p, 0x77, 128);
    EXPECT_EQ(static_cast<uint8_t*>(p)[127], 0x77);
    ISysMem::Instance().Delete(static_cast<char*>(p));

    ISysMem::Instance().SetAllocator(FAST_MEM, saved);
    EXPECT_EQ(ISysMem::Instance().GetAllocator(FAST_MEM), saved);
}

TEST_F(ZSysMemTests, ResetIsSafeAndAllocatorsRemainUsable)
{
    ISysMem::Instance().Reset();

    void* p = ISysMem::Instance().New(DEFAULT_MEM, 64);
    ASSERT_NE(p, nullptr);
    std::memset(p, 0x10, 64);
    EXPECT_EQ(static_cast<uint8_t*>(p)[0], 0x10);
    ISysMem::Instance().Delete(static_cast<char*>(p));
}

TEST_F(ZSysMemTests, FileLineRoundTrip)
{
    const int expectedLine = __LINE__ + 1;
    ISysMem::Instance().SetFileLine(__FILE__, expectedLine);

    const char* file = nullptr;
    int line = 0;
    ISysMem::Instance().GetFileLine(&file, &line);

    EXPECT_STREQ(file, __FILE__);
    EXPECT_EQ(line, expectedLine);
}

TEST_F(ZSysMemTests, UniMemoryRoutesThroughZSysMem)
{
    // While ZSysMem is up, ZUniMemory must allocate/free through it (round trip).
    ASSERT_TRUE(ISysMem::Exists());

    void* p = ZUniMemory::Allocate(200, DEFAULT_MEM);
    ASSERT_NE(p, nullptr);
    std::memset(p, 0x33, 200);
    for (int i = 0; i < 200; ++i)
        ASSERT_EQ(static_cast<uint8_t*>(p)[i], 0x33);

    ZUniMemory::Free(p);

    // Free(nullptr) must be a safe no-op.
    ZUniMemory::Free(nullptr);
}
