#include <Glacier/Animation/Manager.h>
#include <Glacier/Animation/Header.h>
#include <Tests/EngineFixture.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <vector>

using namespace Glacier::Animation;
using Tests::EngineFixture;

namespace
{
    void WriteU32(std::vector<uint8_t>& data, size_t offset, uint32_t value)
    {
        std::memcpy(data.data() + offset, &value, sizeof(value));
    }
}

TEST_F(EngineFixture, AnimationManager_LoadDataBlockMapsPayloadAndNames)
{
    constexpr size_t headerOffset = 0x34;
    constexpr size_t dataOffset = headerOffset + 0x40;
    constexpr size_t boneNamesOffset = dataOffset + 4;
    constexpr size_t animNamesOffset = boneNamesOffset + 6;
    constexpr size_t poseNamesOffset = animNamesOffset + 12;
    std::vector<uint8_t> block(poseNamesOffset + 6, 0);

    WriteU32(block, 0x00, 1);
    WriteU32(block, 0x04, 4);
    WriteU32(block, 0x08, 6);
    WriteU32(block, 0x0C, 12);
    WriteU32(block, 0x10, 6);
    WriteU32(block, 0x14, 1);
    WriteU32(block, 0x18, 2);
    WriteU32(block, 0x1C, 1);
    std::memcpy(block.data() + boneNamesOffset, "root\0\0", 6);
    std::memcpy(block.data() + animNamesOffset, "idle\0run\0\0", 10);
    std::memcpy(block.data() + poseNamesOffset, "pose\0\0", 6);

    Manager manager;
    EXPECT_EQ(manager.LoadDataBlock(block.data(), static_cast<int>(block.size())), 0);
    EXPECT_EQ(manager.m_Headers, reinterpret_cast<Header*>(block.data() + headerOffset));
    EXPECT_EQ(manager.m_Data, block.data() + dataOffset);
    EXPECT_STREQ(manager.GetAnimName(0), "idle");
    EXPECT_STREQ(manager.GetAnimName(1), "run");
    EXPECT_EQ(manager.GetAnimName(2), nullptr);
    EXPECT_EQ(manager.GetBoneID("unused"), static_cast<Glacier::BoneID>(-1));
    EXPECT_EQ(manager.GetPoseID("unused"), static_cast<Glacier::PoseID>(-1));
    EXPECT_EQ(manager.FromIndex(-1), nullptr);
    EXPECT_EQ(manager.FromIndex(1), nullptr);
    EXPECT_EQ(manager.ToIndex(manager.m_Headers), 0);
}

TEST(AnimationManager, MetadataOffsetsAreByteOffsets)
{
    std::vector<uint8_t> metadata(64, 0);
    WriteU32(metadata, 4, 1);
    auto* key = reinterpret_cast<ZMetaKey*>(metadata.data() + 20);
    key->lFrame = 17;
    key->lValue = 42;

    Manager manager;
    manager.m_pMetaKeyData = reinterpret_cast<char*>(metadata.data());

    EXPECT_EQ(manager.GetMetaKeyDataLength(4), 1u);
    EXPECT_EQ(manager.GetMetaKeyData(4), key);
    EXPECT_EQ(manager.GetMetaKeyAtFrame(4, 17), key);
    EXPECT_EQ(manager.GetMetaKeyAtFrame(4, 18), nullptr);
    int32_t index = 0;
    EXPECT_EQ(manager.GetFrameFromMetaValue(4, 42, &index), 17);
    EXPECT_EQ(index, 1);
}

TEST_F(EngineFixture, AnimationManager_ClearResetsBorrowedPayloadState)
{
    std::vector<uint8_t> block(0x34, 0);
    Manager manager;
    manager.LoadDataBlock(block.data(), static_cast<int>(block.size()));

    EXPECT_EQ(manager.Clear(), 0);
    EXPECT_EQ(manager.m_Headers, nullptr);
    EXPECT_EQ(manager.m_Data, nullptr);
    EXPECT_EQ(manager.m_Animcount, 0);
    EXPECT_EQ(manager.m_OwnsBuffers, 0);
}
