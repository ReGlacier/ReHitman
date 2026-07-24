#include <gtest/gtest.h>
#include <Glacier/ZSTL/REFTAB.h>


using namespace Glacier;

class REFTABTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        pReftab = new REFTAB(4, 0); 
    }

    void TearDown() override 
    {
        delete pReftab;
    }

    REFTAB* pReftab = nullptr;
};

TEST_F(REFTABTest, ConstructorInitialization) 
{
    EXPECT_EQ(pReftab->EleCount, 0);
    EXPECT_EQ(pReftab->EleSize, 1);
    EXPECT_EQ(pReftab->BlkSize, 4);
    EXPECT_EQ(pReftab->TabFirstPtr, nullptr);
    EXPECT_EQ(pReftab->TabBlockPtr, nullptr);
}

TEST_F(REFTABTest, InsertAndIterateForward) 
{
    pReftab->Add(10);
    pReftab->Add(20);
    pReftab->Add(30);
    pReftab->Add(40);
    pReftab->Add(50);

    EXPECT_EQ(pReftab->EleCount, 5);
    ASSERT_NE(pReftab->TabFirstPtr, nullptr);
    ASSERT_NE(pReftab->TabBlockPtr, nullptr);
    EXPECT_NE(pReftab->TabFirstPtr, pReftab->TabBlockPtr);

    RefRun it;
    pReftab->RunInitNxtRef(&it);

    uint32_t* pVal = nullptr;
    
    pVal = pReftab->RunNxtRefPtr(&it);
    ASSERT_NE(pVal, nullptr);
    EXPECT_EQ(*pVal, 10);

    pVal = pReftab->RunNxtRefPtr(&it);
    EXPECT_EQ(*pVal, 20);

    pVal = pReftab->RunNxtRefPtr(&it);
    EXPECT_EQ(*pVal, 30);

    pVal = pReftab->RunNxtRefPtr(&it);
    EXPECT_EQ(*pVal, 40);

    pVal = pReftab->RunNxtRefPtr(&it);
    ASSERT_NE(pVal, nullptr);
    EXPECT_EQ(*pVal, 50);

    pVal = pReftab->RunNxtRefPtr(&it);
    EXPECT_EQ(pVal, nullptr);
}

TEST_F(REFTABTest, IterateBackward) 
{
    pReftab->Add(100);
    pReftab->Add(200);
    pReftab->Add(300);
    pReftab->Add(400);
    pReftab->Add(500);

    RefRun it;
    pReftab->RunInitPrevRef(&it);

    uint32_t* pVal = pReftab->RunPrevRefPtr(&it);
    ASSERT_NE(pVal, nullptr);
    EXPECT_EQ(*pVal, 500);

    pVal = pReftab->RunPrevRefPtr(&it);
    ASSERT_NE(pVal, nullptr);
    EXPECT_EQ(*pVal, 400);

    pVal = pReftab->RunPrevRefPtr(&it);
    EXPECT_EQ(*pVal, 300);

    pVal = pReftab->RunPrevRefPtr(&it);
    EXPECT_EQ(*pVal, 200);

    pVal = pReftab->RunPrevRefPtr(&it);
    EXPECT_EQ(*pVal, 100);

    pVal = pReftab->RunPrevRefPtr(&it);
    EXPECT_EQ(pVal, nullptr);
}

TEST_F(REFTABTest, DeleteFromMiddleReplacesWithLast) 
{
    pReftab->Add(10);
    pReftab->Add(20);
    pReftab->Add(30);

    RefRun it;
    pReftab->RunInitNxtRef(&it);
    pReftab->RunNxtRefPtr(&it);
    pReftab->RunNxtRefPtr(&it);

    pReftab->RunDelRef(&it);

    EXPECT_EQ(pReftab->EleCount, 2);

    pReftab->RunInitNxtRef(&it);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 10);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 30);
}

TEST_F(REFTABTest, DeleteVeryLastElementCleansUp) 
{
    pReftab->Add(999);

    RefRun it;
    pReftab->RunInitNxtRef(&it);
    pReftab->RunNxtRefPtr(&it);
    pReftab->RunDelRef(&it);

    EXPECT_EQ(pReftab->EleCount, 0);
    EXPECT_EQ(pReftab->TabFirstPtr, nullptr);
    EXPECT_EQ(pReftab->TabBlockPtr, nullptr);
    EXPECT_EQ(it._RunPtr, nullptr);
}

TEST_F(REFTABTest, ClearEmptiesContainer) 
{
    pReftab->Add(10);
    pReftab->Add(20);
    pReftab->Add(30);
    pReftab->Add(40);
    pReftab->Add(50);

    pReftab->Clear();

    EXPECT_EQ(pReftab->EleCount, 0);
    EXPECT_EQ(pReftab->TabFirstPtr, nullptr);
    EXPECT_EQ(pReftab->TabBlockPtr, nullptr);

    pReftab->Add(99);
    EXPECT_EQ(pReftab->EleCount, 1);
    ASSERT_NE(pReftab->TabFirstPtr, nullptr);
    
    RefRun it;
    pReftab->RunInitNxtRef(&it);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 99);
}

TEST_F(REFTABTest, AssertionsOnInvalidInputs) 
{
    EXPECT_THROW(pReftab->RunInitNxtRef(nullptr), std::runtime_error);
    EXPECT_THROW(pReftab->RunInitPrevRef(nullptr), std::runtime_error);
    EXPECT_THROW(pReftab->RunNxtRefPtr(nullptr), std::runtime_error);
    EXPECT_THROW(pReftab->RunPrevRefPtr(nullptr), std::runtime_error);
    EXPECT_THROW(pReftab->RunToRefPtr(nullptr), std::runtime_error);

    RefRun emptyIt;
    EXPECT_THROW(pReftab->RunDelRef(&emptyIt), std::runtime_error);
}

TEST_F(REFTABTest, DeleteFirstElementReplacesWithLast) 
{
    pReftab->Add(10);
    pReftab->Add(20);
    pReftab->Add(30);
    pReftab->Add(40);

    RefRun it;
    pReftab->RunInitNxtRef(&it);
    pReftab->RunNxtRefPtr(&it);

    pReftab->RunDelRef(&it);

    EXPECT_EQ(pReftab->EleCount, 3);

    pReftab->RunInitNxtRef(&it);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 40);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 20);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 30);
}

TEST_F(REFTABTest, DeleteElementCrossBlock) 
{
    pReftab->Add(1);
    pReftab->Add(2);
    pReftab->Add(3);
    pReftab->Add(4);
    pReftab->Add(5);
    pReftab->Add(6);

    RefRun it;
    pReftab->RunInitNxtRef(&it);
    pReftab->RunNxtRefPtr(&it);
    pReftab->RunNxtRefPtr(&it);
    pReftab->RunNxtRefPtr(&it);

    pReftab->RunDelRef(&it);

    EXPECT_EQ(pReftab->EleCount, 5);

    pReftab->RunInitNxtRef(&it);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 1);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 2);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 6);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 4);
    EXPECT_EQ(*pReftab->RunNxtRefPtr(&it), 5);
    EXPECT_EQ(pReftab->RunNxtRefPtr(&it), nullptr);
}

TEST_F(REFTABTest, IndexOperatorAccess) 
{
    for (uint32_t i = 0; i < 8; ++i) 
    {
        pReftab->Add((i + 1) * 10);
    }

    EXPECT_EQ((*pReftab)[0], 10);
    EXPECT_EQ((*pReftab)[3], 40);
    EXPECT_EQ((*pReftab)[4], 50);
    EXPECT_EQ((*pReftab)[7], 80);
    EXPECT_EQ((*pReftab)[8], 0);
    EXPECT_EQ((*pReftab)[1000], 0);
    EXPECT_THROW((*pReftab)[-1], std::runtime_error);
}

TEST_F(REFTABTest, DestructorCleansUpMemory) 
{
    {
        REFTAB localReftab(4, 3);

        for (int i = 0; i < 100; i++)
        {
            localReftab.Add(i+1);
        }
        
        EXPECT_EQ(localReftab.EleCount, 100);
    }
    SUCCEED(); 
}

TEST_F(REFTABTest, RangeBasedForLoop) 
{
    REFTAB reftab{4,3};
    reftab.Add(11);
    reftab.Add(22);
    reftab.Add(33);
    reftab.Add(44);
    reftab.Add(55);

    std::vector<uint32_t> extractedValues;

    for (auto value : reftab) 
    {
        extractedValues.push_back(value);
    }

    ASSERT_EQ(extractedValues.size(), 5);
    EXPECT_EQ(extractedValues[0], 11);
    EXPECT_EQ(extractedValues[1], 22);
    EXPECT_EQ(extractedValues[2], 33);
    EXPECT_EQ(extractedValues[3], 44);
    EXPECT_EQ(extractedValues[4], 55);
}

TEST_F(REFTABTest, TypedViewIteratesPointerValues)
{
    auto* first = reinterpret_cast<int*>(static_cast<uintptr_t>(0x1000));
    auto* second = reinterpret_cast<int*>(static_cast<uintptr_t>(0x2000));
    auto* third = reinterpret_cast<int*>(static_cast<uintptr_t>(0x3000));

    pReftab->Add(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(first)));
    pReftab->Add(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(second)));
    pReftab->Add(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(third)));

    std::vector<int*> extractedPointers;
    for (auto* value : pReftab->As<int*>())
    {
        extractedPointers.push_back(value);
    }

    ASSERT_EQ(extractedPointers.size(), 3);
    EXPECT_EQ(extractedPointers[0], first);
    EXPECT_EQ(extractedPointers[1], second);
    EXPECT_EQ(extractedPointers[2], third);
}

TEST_F(REFTABTest, TypedViewDoesNotChangeDefaultIteration)
{
    pReftab->Add(11);
    pReftab->Add(22);
    pReftab->Add(33);

    std::vector<uint32_t> defaultValues;
    for (auto value : *pReftab)
    {
        defaultValues.push_back(value);
    }

    std::vector<uint16_t> typedValues;
    for (auto value : pReftab->As<uint16_t>())
    {
        typedValues.push_back(value);
    }

    ASSERT_EQ(defaultValues.size(), 3);
    EXPECT_EQ(defaultValues[0], 11);
    EXPECT_EQ(defaultValues[1], 22);
    EXPECT_EQ(defaultValues[2], 33);

    ASSERT_EQ(typedValues.size(), 3);
    EXPECT_EQ(typedValues[0], 11);
    EXPECT_EQ(typedValues[1], 22);
    EXPECT_EQ(typedValues[2], 33);
}

TEST_F(REFTABTest, CustomElementSizeHandling) 
{
    auto* pCustomTab = new REFTAB(4, 1); 

    EXPECT_EQ(pCustomTab->EleCount, 0);
    EXPECT_EQ(pCustomTab->EleSize, 2);
    EXPECT_EQ(pCustomTab->BlkSize, 8);

    uint32_t* pSlot1 = pCustomTab->Add(0x11111111);
    ASSERT_NE(pSlot1, nullptr);
    *pSlot1 = 0x22222222;

    uint32_t* pSlot2 = pCustomTab->Add(0x33333333);
    ASSERT_NE(pSlot2, nullptr);
    *pSlot2 = 0x44444444;

    EXPECT_EQ(pCustomTab->EleCount, 2);

    uint32_t* pStartOfElement1 = pSlot1 - 1;
    uint32_t* pStartOfElement2 = pSlot2 - 1;
    
    EXPECT_EQ(pStartOfElement1 + pCustomTab->EleSize, pStartOfElement2);

    RefRun it;
    pCustomTab->RunInitNxtRef(&it);

    uint32_t* pRead1 = pCustomTab->RunNxtRefPtr(&it);
    ASSERT_NE(pRead1, nullptr);
    EXPECT_EQ(pRead1[0], 0x11111111);
    EXPECT_EQ(pRead1[1], 0x22222222);

    uint32_t* pRead2 = pCustomTab->RunNxtRefPtr(&it);
    ASSERT_NE(pRead2, nullptr);
    EXPECT_EQ(pRead2[0], 0x33333333);
    EXPECT_EQ(pRead2[1], 0x44444444);

    EXPECT_EQ(pCustomTab->RunNxtRefPtr(&it), nullptr);

    delete pCustomTab;
}
