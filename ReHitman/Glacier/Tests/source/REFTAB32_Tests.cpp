#include <gtest/gtest.h>
#include <Glacier/ZSTL/REFTAB32.h>


using namespace Glacier;

class REFTAB32Test : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        pReftab = new REFTAB32(); 
    }

    void TearDown() override 
    {
        delete pReftab;
    }

    REFTAB32* pReftab = nullptr;
};

TEST_F(REFTAB32Test, ConstructorInitialization) 
{
    EXPECT_EQ(pReftab->EleCount, 0);
    EXPECT_EQ(pReftab->m_lRefsPrBlk, 32);
    EXPECT_EQ(pReftab->EleSize, 1);
    EXPECT_EQ(pReftab->BlkSize, 32);
    EXPECT_EQ(pReftab->TabFirstPtr, nullptr);
    EXPECT_EQ(pReftab->TabBlockPtr, nullptr);
}

TEST_F(REFTAB32Test, SmallBufferOptimizationInternalStorage) 
{
    for (int i = 0; i < 32; ++i) 
    {
        pReftab->Add(i * 10);
    }

    EXPECT_EQ(pReftab->EleCount, 32);
    ASSERT_NE(pReftab->TabFirstPtr, nullptr);
    
    EXPECT_EQ(pReftab->TabFirstPtr, reinterpret_cast<TabBlk*>(pReftab->m_FirstTab));
    EXPECT_EQ(pReftab->TabBlockPtr, reinterpret_cast<TabBlk*>(pReftab->m_FirstTab));
}

TEST_F(REFTAB32Test, OverflowTriggersDynamicAllocation) 
{
    for (int i = 0; i < 32; ++i) 
    {
        pReftab->Add(i);
    }

    pReftab->Add(999);

    EXPECT_EQ(pReftab->EleCount, 33);
    
    EXPECT_EQ(pReftab->TabFirstPtr, reinterpret_cast<TabBlk*>(pReftab->m_FirstTab));
    
    ASSERT_NE(pReftab->TabBlockPtr, nullptr);
    EXPECT_NE(pReftab->TabBlockPtr, reinterpret_cast<TabBlk*>(pReftab->m_FirstTab));
}

TEST_F(REFTAB32Test, RangeBasedForLoopCompatibility) 
{
    pReftab->Add(5);
    pReftab->Add(10);
    pReftab->Add(15);

    int sum = 0;
    
    for (uint32_t value : *pReftab) 
    {
        sum += value;
    }

    EXPECT_EQ(sum, 30);
}

TEST_F(REFTAB32Test, NativeGlacierIterationLoop) 
{
    pReftab->Add(5);
    pReftab->Add(10);
    pReftab->Add(15);

    EXPECT_EQ(pReftab->EleCount, 3);

    RefRun it;
    pReftab->RunInitNxtRef(&it);

    uint32_t* pVal1 = pReftab->RunNxtRefPtr(&it);
    ASSERT_NE(pVal1, nullptr) << "First element pointer is null!";
    std::cout << "[Native Test] Element 1 Address: " << pVal1 << ", Value: " << *pVal1 << std::endl;

    uint32_t* pVal2 = pReftab->RunNxtRefPtr(&it);
    ASSERT_NE(pVal2, nullptr) << "Second element pointer is null!";
    std::cout << "[Native Test] Element 2 Address: " << pVal2 << ", Value: " << *pVal2 << std::endl;

    uint32_t* pVal3 = pReftab->RunNxtRefPtr(&it);
    ASSERT_NE(pVal3, nullptr) << "Third element pointer is null!";
    std::cout << "[Native Test] Element 3 Address: " << pVal3 << ", Value: " << *pVal3 << std::endl;

    uint32_t* pEnd = pReftab->RunNxtRefPtr(&it);
    EXPECT_EQ(pEnd, nullptr);

    EXPECT_EQ(*pVal1, 5);
    EXPECT_EQ(*pVal2, 10);
    EXPECT_EQ(*pVal3, 15);

    int nativeSum = *pVal1 + *pVal2 + *pVal3;
    EXPECT_EQ(nativeSum, 30);
}