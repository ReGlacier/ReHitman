#include <Glacier/ZSTL/ALLOCREF.h>
#include <Glacier/ZSTL/REFTAB.h>

#include <gtest/gtest.h>
#include <vector>

using namespace Glacier;


TEST(ALLOCREF_Tests, InitializationAndInitialState)
{
    Glacier::ALLOCREF pool(16, 8);

    EXPECT_EQ(pool.m_lRefShift, 16);
    EXPECT_EQ(pool.m_lRefAnd, 7);
    EXPECT_EQ(pool.m_lActRefNr, 0);
    EXPECT_EQ(pool.GetNrActiveRefs(), 0);

    unsigned int* usedRefs = pool.GetUsedRefs();
    ASSERT_NE(usedRefs, nullptr);
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(usedRefs[i], 1 << 16);
    }
}

TEST(ALLOCREF_Tests, SequentialNewRefAllocation)
{
    Glacier::ALLOCREF pool(16, 4);

    Glacier::ZREF ref0 = pool.NewRef();

    EXPECT_EQ(ref0, 0x00010000);
    EXPECT_EQ(pool.GetNrActiveRefs(), 1);
    EXPECT_TRUE(pool.CheckRefActive(ref0));

    Glacier::ZREF ref1 = pool.NewRef();

    EXPECT_EQ(ref1, 0x00010001);
    EXPECT_EQ(pool.GetNrActiveRefs(), 2);
    EXPECT_TRUE(pool.CheckRefActive(ref1));

    EXPECT_FALSE(pool.CheckRefActive(0x00020002));
}

TEST(ALLOCREF_Tests, FreeRefAndActivityChecks)
{
    Glacier::ALLOCREF pool(16, 4);

    Glacier::ZREF ref0 = pool.NewRef();
    Glacier::ZREF ref1 = pool.NewRef();

    EXPECT_TRUE(pool.FreeRef(ref0));
    EXPECT_EQ(pool.GetNrActiveRefs(), 1);
    
    EXPECT_FALSE(pool.CheckRefActive(ref0));

    EXPECT_FALSE(pool.FreeRef(ref0));

    EXPECT_FALSE(pool.FreeRef(0x99999999));
}

TEST(ALLOCREF_Tests, RefRecyclingWithGenerationIncrement)
{
    Glacier::ALLOCREF pool(16, 4);

    Glacier::ZREF ref0 = pool.NewRef();
    Glacier::ZREF ref1 = pool.NewRef();
    
    EXPECT_EQ(ref0, 0x00010000);

    EXPECT_TRUE(pool.FreeRef(ref0));

    Glacier::ZREF recycledRef0 = pool.NewRef();

    EXPECT_EQ(recycledRef0, 0x00020000);
    EXPECT_TRUE(pool.CheckRefActive(recycledRef0));
    
    EXPECT_FALSE(pool.CheckRefActive(ref0)); 
}

TEST(ALLOCREF_Tests, GetActiveRefsPopulatesReftab)
{
    Glacier::ALLOCREF pool(16, 4);

    Glacier::ZREF r0 = pool.NewRef();
    Glacier::ZREF r1 = pool.NewRef();
    Glacier::ZREF r2 = pool.NewRef();

    pool.FreeRef(r1);

    Glacier::REFTAB* activeTable = Glacier::REFTAB::MakeReftab(10, 0);
    
    uint32_t activeCount = pool.GetActiveRefs(activeTable);
    EXPECT_EQ(activeCount, 2);
    EXPECT_EQ(activeTable->Count(), 2);

    std::vector<uint32_t> collectedRefs;
    Glacier::RefRun it;
    activeTable->RunInitNxtRef(&it);
    for (uint32_t val = activeTable->RunNxtRef(&it); it; val = activeTable->RunNxtRef(&it))
    {
        collectedRefs.push_back(val);
    }

    EXPECT_EQ(collectedRefs.size(), 2);
    
    EXPECT_EQ(collectedRefs[0], r0);
    EXPECT_EQ(collectedRefs[1], r2);

    Glacier::REFTAB::DeleteReftab(activeTable);
}

TEST(ALLOCREF_Tests, OutOfPoolTriggersAssert)
{
    Glacier::ALLOCREF pool(16, 2); 

    pool.NewRef();
    pool.NewRef();

    EXPECT_THROW(pool.NewRef(), std::runtime_error);
}

TEST(ALLOCREF_Tests, StateRestorationViaSetters)
{
    Glacier::ALLOCREF pool(16, 4);

    std::vector<Glacier::ZREF> customUsedRefs = 
    {
        0x000A0000,
        0x000C0000,
        0x000C0000,
        0x000E0000 
    };
    pool.SetUsedRefs(customUsedRefs.data(), 4);

    Glacier::REFTAB* fakeFreeStack = Glacier::REFTAB::MakeReftab(10, 0);
    fakeFreeStack->Add(2);
    fakeFreeStack->Add(4);
    
    pool.SetRefStack(fakeFreeStack);
    pool.m_lActRefNr = 4;

    EXPECT_FALSE(pool.CheckRefActive(0x000B0001));
    EXPECT_TRUE(pool.CheckRefActive(0x000A0000)); 

    Glacier::ZREF recycled = pool.NewRef();
    
    EXPECT_EQ(recycled, 0x000C0001);

    Glacier::REFTAB::DeleteReftab(fakeFreeStack);
}