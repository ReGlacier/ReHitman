// Reference-data validation for ZSharedResourcesD3D runtime data.
//
// The bundled test data under Tests/data is produced by the reference
// tooling (Tools/ida_dump); the tests auto-skip when a fixture is absent.
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace
{
    std::vector<uint8_t> LoadFixture(const char* pszName)
    {
        const std::filesystem::path kPath =
            std::filesystem::path(GLACIER_TESTS_DATA_DIR) / pszName;
        std::ifstream stream(kPath, std::ios::binary);
        if (!stream)
        {
            return {};
        }
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)),
            std::istreambuf_iterator<char>());
    }

    uint32_t ReadU32(const std::vector<uint8_t>& rData, size_t lOffset)
    {
        uint32_t lValue = 0;
        std::memcpy(&lValue, rData.data() + lOffset, sizeof(lValue));
        return lValue;
    }

    uint32_t FloatBits(float fValue)
    {
        uint32_t lBits = 0;
        std::memcpy(&lBits, &fValue, sizeof(lBits));
        return lBits;
    }

    bool HasBytes(const std::vector<uint8_t>& rData, size_t lOffset, const char* pszText)
    {
        const size_t lLen = std::strlen(pszText);
        return lOffset + lLen <= rData.size() && std::memcmp(rData.data() + lOffset, pszText, lLen) == 0;
    }
}

// The PC 8x13 fixed-font writer keeps one float pair per ASCII code at
// 0x008EE130: u = (8*(c & 0xF))/128, v = (13*(c / 16))/128 (PC Allocate
// 0x0048A50C/0x0048A52C). Our ZSharedResourcesD3D::Allocate fills the same
// table; the reference below is taken from the bundled fixture.
TEST(ZSharedResourcesD3D, FontLetterPositionsMatchReference)
{
    const std::vector<uint8_t> kData = LoadFixture("font_letter_positions.bin");
    if (kData.empty())
    {
        GTEST_SKIP() << "fixture Tests/data/font_letter_positions.bin is not available";
    }
    ASSERT_GE(kData.size(), 0x400u);

    for (int lChar = 0x20; lChar < 0x7F; ++lChar)
    {
        const size_t lOffset = static_cast<size_t>(lChar) * 8;
        const float kExpectedU = static_cast<float>(8 * (lChar & 0xF)) * (1.0f / 128.0f);
        const float kExpectedV = static_cast<float>(13 * (lChar / 16)) * (1.0f / 128.0f);
        EXPECT_EQ(ReadU32(kData, lOffset + 0), FloatBits(kExpectedU)) << "u of char 0x" << std::hex << lChar;
        EXPECT_EQ(ReadU32(kData, lOffset + 4), FloatBits(kExpectedV)) << "v of char 0x" << std::hex << lChar;
    }
}

// Smoke check of the four container-registered textures after Allocate: every
// wrapper must hold a non-null D3D object (m_pUserData, ZTextureBase +0x2C) and
// the expected custom name (ZTextureBase::m_szName at +0x14).
TEST(ZSharedResourcesD3D, ContainerTextureWrappersMatchReference)
{
    struct SEntry
    {
        const char* pszFile;
        const char* pszName;
    };
    // NOTE: fixture file names keep the historical global names. The bundled
    // reference data shows the real mapping is 0x90AE58 = ShadowClip1x2 and
    // 0x90AEA0 = Font8x13, so the name asserted below is the texture stored at
    // that wrapper's address.
    const SEntry kEntries[] = {
        { "g_texWhite.bin", "White1x1" },
        { "g_texMouse16x16.bin", "Mouse16x16" },
        { "g_texFont8x13.bin", "ShadowClip1x2" },      // file=address 0x90AE58
        { "g_texShadowClip1x2.bin", "Font8x13" },      // file=address 0x90AEA0
    };

    bool bMissing = false;
    for (const SEntry& rEntry : kEntries)
    {
        const std::vector<uint8_t> kData = LoadFixture(rEntry.pszFile);
        if (kData.empty())
        {
            bMissing = true;
            continue;
        }
        ASSERT_GE(kData.size(), 0x44u) << rEntry.pszFile;
        EXPECT_NE(ReadU32(kData, 0x2C), 0u) << rEntry.pszFile << " m_pUserData";
        EXPECT_TRUE(HasBytes(kData, 0x14, rEntry.pszName)) << rEntry.pszFile << " m_szName";
    }
    if (bMissing)
    {
        GTEST_SKIP() << "one or more wrapper fixtures under Tests/data are not available";
    }
}
