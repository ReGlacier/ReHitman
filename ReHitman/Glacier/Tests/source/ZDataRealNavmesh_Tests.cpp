#include <Glacier/PF4/ZData.h>
#include <Glacier/PF4/ZPath.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace Glacier;
using namespace Glacier::PF4;

namespace
{
    constexpr const char* kXorKey = "ReHitmanT00l$2k26";

    // Reads + de-obfuscates a dumped PF4 payload. The on-disk format is
    // XOR(payload) with payload = [u32 size][pathfinder data block], matching the
    // GMS chunk layout consumed by ZEngineDataBase::InitPathfinder4Data.
    bool ReadPathfinder4Dump(std::vector<uint8_t>& outPayload)
    {
        const std::filesystem::path kDataPath =
            std::filesystem::path(GLACIER_TESTS_DATA_DIR) / "PF01.bin";
        std::ifstream stream(kDataPath, std::ios::binary);
        if (!stream)
        {
            return false;
        }

        std::vector<uint8_t> encrypted((std::istreambuf_iterator<char>(stream)),
            std::istreambuf_iterator<char>());
        if (encrypted.empty())
        {
            return false;
        }

        const size_t uKeyLen = std::strlen(kXorKey);
        outPayload.resize(encrypted.size());
        for (size_t i = 0; i < encrypted.size(); ++i)
        {
            outPayload[i] = static_cast<uint8_t>(encrypted[i] ^ kXorKey[i % uKeyLen]);
        }
        return true;
    }
}

// Runs the pathfinding entry points against a real (obfuscated) PF4 dump bundled
// under Tests/data. The dump only contains the navigation chunk itself, so no
// level assets are stored in the repository.
TEST(ZDataRealNavmesh, LoadsDumpAndServesPathQueries)
{
    std::vector<uint8_t> payload;
    if (!ReadPathfinder4Dump(payload))
    {
        GTEST_SKIP() << "PF4 test dump (Tests/data/PF01.bin) is not available.";
    }

    ASSERT_GE(payload.size(), 4u);
    const uint32_t lDataSize = *reinterpret_cast<const uint32_t*>(payload.data());
    ASSERT_GT(lDataSize, 0u);
    ASSERT_LE(lDataSize, payload.size() - 4u);

    // The data block starts right after the size field and must stay alive for
    // as long as the ZData object references it.
    std::vector<uint8_t> block(payload.begin() + 4, payload.begin() + 4 + lDataSize);

    ZData pathFinder;
    pathFinder.LoadDataBlock(block.data());

    EXPECT_GT(pathFinder.GraphCount(), 0);
    EXPECT_GT(pathFinder.ComponentCount(), 0);
    EXPECT_GT(pathFinder.NodeCount(), 0);
    EXPECT_GT(pathFinder.VertexCount(), 0);

    // We do not know the level layout, so scan world positions taken from the
    // navmesh vertices until two mapped locations on the same graph are found.
    struct Key
    {
        int iGraph;
        int iComponent;
        bool operator==(const Key& rhs) const
        {
            return iGraph == rhs.iGraph && iComponent == rhs.iComponent;
        }
    };

    std::array<Key, 2> aFound;
    std::array<ZLocation, 2> aLocations;
    int iFoundCount = 0;
    const int iVertexCount = pathFinder.VertexCount();
    for (int i = 0; i < iVertexCount && iFoundCount < 2; i += 7)
    {
        const ZVertex& vertex = pathFinder.m_pVertices[i];
        const ZVector3 vProbe(vertex.m_kPos.x, vertex.m_fHeight, vertex.m_kPos.y);

        ZLocation location;
        if (!pathFinder.MapValidLocation(vProbe, location) || location.Graph() == -1
            || location.Component() == -1)
        {
            continue;
        }

        const Key key{ location.Graph(), location.Component() };
        bool bDuplicate = false;
        for (int k = 0; k < iFoundCount; ++k)
        {
            if (aFound[k] == key)
            {
                bDuplicate = true;
                break;
            }
        }
        if (bDuplicate)
        {
            continue;
        }

        // Prefer a second location on the same graph as the first one.
        if (iFoundCount > 0 && key.iGraph != aFound[0].iGraph)
        {
            continue;
        }
        aFound[iFoundCount] = key;
        aLocations[iFoundCount] = location;
        ++iFoundCount;
    }

    ASSERT_GE(iFoundCount, 1) << "No navmesh location could be mapped in the dump";
    if (iFoundCount < 2)
    {
        GTEST_SKIP() << "Only one component reachable in the dump; skipping cross-component check.";
    }

    const ZLocation& location = aLocations[0];
    const ZLocation& destination = aLocations[1];

    // The trivial query (start == goal) must yield the two-link straight path.
    ZDataRef aRefs[512];
    ZPath path(aRefs, 512);
    path.m_PathFinder = &pathFinder;
    ZPathLink aLinks[64];
    std::memset(aLinks, 0, sizeof(aLinks));

    const int iSameComponentLinks = pathFinder.FindComponentPathAStar(
        location, location, path, aLinks, 0xFFFFFFFF);
    EXPECT_EQ(iSameComponentLinks, 2);
    EXPECT_EQ(path.m_Size, 2);

    // A cross-component query inside the same graph must terminate and, when the
    // components are connected, produce a real path ending at the destination.
    ZDataRef aRefs2[1024];
    ZPath path2(aRefs2, 1024);
    path2.m_PathFinder = &pathFinder;
    ZPathLink aLinks2[256];
    std::memset(aLinks2, 0, sizeof(aLinks2));

    const int iPathLinks = pathFinder.FindComponentPathAStar(
        location, destination, path2, aLinks2, 0xFFFFFFFF);
    ASSERT_GE(iPathLinks, 2);
    EXPECT_GE(path2.m_Size, 2);
    EXPECT_LE(iPathLinks, 256);

    // All emitted positions must be finite, the first one the start point and the
    // last one the mapped destination.
    EXPECT_FLOAT_EQ(aLinks2[0].m_Pos.x, location.m_vPos.x);
    EXPECT_FLOAT_EQ(aLinks2[0].m_Pos.z, location.m_vPos.z);
    for (int i = 0; i < iPathLinks; ++i)
    {
        EXPECT_TRUE(std::isfinite(aLinks2[i].m_Pos.x));
        EXPECT_TRUE(std::isfinite(aLinks2[i].m_Pos.y));
        EXPECT_TRUE(std::isfinite(aLinks2[i].m_Pos.z));
    }
    EXPECT_NEAR(aLinks2[iPathLinks - 1].m_Pos.x, destination.m_vPos.x, 0.01f);
    EXPECT_NEAR(aLinks2[iPathLinks - 1].m_Pos.z, destination.m_vPos.z, 0.01f);
    EXPECT_GE(path2.m_Cost, 0.0f);
}
