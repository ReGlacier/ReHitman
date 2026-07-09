#include <gtest/gtest.h>
#include <Glacier/CHUNKFILE.h>
#include <cstdint>


constexpr uint32_t FOURCC_ROOT = 0x544F4F52; // 'ROOT'
constexpr uint32_t FOURCC_DATA = 0x41544144; // 'DATA'
constexpr uint32_t FOURCC_SUB1 = 0x31425553; // 'SUB1'
constexpr uint32_t FOURCC_SUB2 = 0x32425553; // 'SUB2'

using namespace Glacier;


TEST(ChunkFileTest, SimpleFlatChunk) 
{
    ChunkBuilder builder(FOURCC_DATA);
    std::vector<uint8_t> dummyData = { 0xAA, 0xBB, 0xCC, 0xDD };
    builder.SetData(dummyData.data(), dummyData.size());

    std::vector<uint8_t> buffer = builder.Serialize();
    CHUNKFILE* chunk = reinterpret_cast<CHUNKFILE*>(buffer.data());

    EXPECT_EQ(chunk->Name, FOURCC_DATA);    
    EXPECT_EQ(chunk->m_lTotalSize & 0x3FFFFFFF, 12);
    EXPECT_GE(chunk->m_lTotalSize, 0);
    EXPECT_EQ(chunk->DataSize(), 4);
    
    uint8_t* rawDataPtr = reinterpret_cast<uint8_t*>(chunk->Data());
    EXPECT_EQ(rawDataPtr[0], 0xAA);
    EXPECT_EQ(rawDataPtr[3], 0xDD);
}

TEST(ChunkFileTest, ContainerWithChildren) 
{
    ChunkBuilder rootBuilder(FOURCC_ROOT);
    
    auto& child1 = rootBuilder.AddChild(FOURCC_SUB1);
    std::vector<uint8_t> data1 = { 0x11, 0x11 };
    child1.SetData(data1.data(), data1.size());

    auto& child2 = rootBuilder.AddChild(FOURCC_SUB2);
    std::vector<uint8_t> data2 = { 0x22, 0x22, 0x22 };
    child2.SetData(data2.data(), data2.size());

    std::vector<uint8_t> buffer = rootBuilder.Serialize();
    CHUNKFILE* rootChunk = reinterpret_cast<CHUNKFILE*>(buffer.size() ? buffer.data() : nullptr);

    ASSERT_NE(rootChunk, nullptr);
    EXPECT_EQ(rootChunk->Name, FOURCC_ROOT);
    EXPECT_LT(rootChunk->m_lTotalSize, 0);

    CHUNKFILE* foundChild1 = rootChunk->FindChild(FOURCC_SUB1);
    ASSERT_NE(foundChild1, nullptr);
    EXPECT_EQ(foundChild1->Name, FOURCC_SUB1);
    EXPECT_EQ(foundChild1->DataSize(), 2);

    CHUNKFILE* foundChild2 = rootChunk->FindChild(FOURCC_SUB2);
    ASSERT_NE(foundChild2, nullptr);
    EXPECT_EQ(foundChild2->Name, FOURCC_SUB2);
    EXPECT_EQ(foundChild2->DataSize(), 3);

    CHUNKFILE* fakeChild = rootChunk->FindChild(0xDEADC0DE);
    EXPECT_EQ(fakeChild, nullptr);
}

TEST(ChunkFileTest, ComplexType3Chunk) 
{
    ChunkBuilder rootBuilder(FOURCC_ROOT);

    uint32_t piece1 = 0x11223344;
    uint16_t piece2 = 0x5566;
    rootBuilder.AddDataPiece(&piece1, sizeof(piece1));
    rootBuilder.AddDataPiece(&piece2, sizeof(piece2));

    auto& child = rootBuilder.AddChild(FOURCC_SUB1);
    uint8_t childData = 0x99;
    child.SetData(&childData, sizeof(childData));

    std::vector<uint8_t> buffer = rootBuilder.Serialize();
    CHUNKFILE* rootChunk = reinterpret_cast<CHUNKFILE*>(buffer.data());

    uint32_t flags = rootChunk->m_lTotalSize & 0xC0000000;
    EXPECT_EQ(flags, 0xC0000000); 

    uint32_t* parsedPiece1 = reinterpret_cast<uint32_t*>(rootChunk->Data());
    EXPECT_EQ(*parsedPiece1, 0x11223344);

    CHUNKFILE* foundChild = rootChunk->FindChild(FOURCC_SUB1);
    ASSERT_NE(foundChild, nullptr);
    EXPECT_EQ(foundChild->Name, FOURCC_SUB1);
    
    uint8_t* childDataPtr = reinterpret_cast<uint8_t*>(foundChild->Data());
    EXPECT_EQ(*childDataPtr, 0x99);
}