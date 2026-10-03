#include <Glacier/Render/Bitmap/ZBitmapPalOpac.h>
#include <gtest/gtest.h>

#include <vector>

using namespace Glacier;

namespace
{
    constexpr int kPalSize = 256;

    // Fills a palette with distinct easily recognizable colors (alpha bits set on purpose).
    void MakePalette(uint32_t* pPal, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            pPal[i] = 0xAA000000u | (static_cast<uint32_t>(i) << 16) | (static_cast<uint32_t>(i) << 8) | static_cast<uint32_t>(i);
        }
    }

    // Builds a paletted bitmap with an aliased (not owned) opacity buffer.
    void MakePalOpacBitmap(ZBitmapPalOpac& bmp, const uint8_t* indices, int sx, int sy, uint32_t* pPal, int palSize, uint8_t* pOpac)
    {
        bmp.SetSizeX(sx);
        bmp.SetSizeY(sy);
        bmp.SetMIPLevels(1);
        bmp.SetData(const_cast<uint8_t*>(indices), sx * sy, sx, sy, 0, true);
        bmp.SetPalette(pPal, palSize, true);
        bmp.m_pOpac = reinterpret_cast<uint32_t*>(pOpac);
    }
}

TEST(ZBitmapPalOpac, CtorSetsTypeAndDefaults)
{
    ZBitmapPalOpac bmp;

    EXPECT_EQ(bmp.GetType(), 1346456655); // 'PALO'
    EXPECT_EQ(bmp.GetSizeX(), 0);
    EXPECT_EQ(bmp.GetSizeY(), 0);
    EXPECT_EQ(bmp.GetMipLevelCount(), 0);
    EXPECT_FLOAT_EQ(bmp.GetScaleFactor(), 1.0f);
    EXPECT_EQ(bmp.GetPal(), nullptr);
    EXPECT_EQ(bmp.GetPalSize(), 0);
    EXPECT_EQ(bmp.m_pOpac, nullptr);
    EXPECT_FALSE(bmp.m_bOpacAllocated);
}

TEST(ZBitmapPalOpac, GetRGBACombinesPaletteWithOpacity)
{
    const uint8_t indices[4] = { 0, 1, 2, 3 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);
    uint8_t opac[4] = { 0xFF, 0x80, 0x00, 0x40 };

    ZBitmapPalOpac bmp;
    MakePalOpacBitmap(bmp, indices, 2, 2, pal, kPalSize, opac);

    uint8_t dst[32] = {};
    bmp.GetRGBA(dst, 0, 0, 2, 2, 16, 0);

    const auto* pRow0 = reinterpret_cast<const uint32_t*>(dst);
    const auto* pRow1 = reinterpret_cast<const uint32_t*>(dst + 16);
    // Palette alpha (0xAA) is masked off; alpha comes from the opacity buffer.
    EXPECT_EQ(pRow0[0], (pal[0] & 0xFFFFFFu) | 0xFF000000u);
    EXPECT_EQ(pRow0[1], (pal[1] & 0xFFFFFFu) | 0x80000000u);
    EXPECT_EQ(pRow1[0], (pal[2] & 0xFFFFFFu) | 0x00000000u);
    EXPECT_EQ(pRow1[1], (pal[3] & 0xFFFFFFu) | 0x40000000u);
}

TEST(ZBitmapPalOpac, GetRGBAReadsSubRect)
{
    const uint8_t indices[4] = { 0, 1, 2, 3 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);
    uint8_t opac[4] = { 0x11, 0x22, 0x33, 0x44 };

    ZBitmapPalOpac bmp;
    MakePalOpacBitmap(bmp, indices, 2, 2, pal, kPalSize, opac);

    // Unlike ZBitmapPal, the Opac variant does not assert on partial reads.
    uint8_t dst[4] = {};
    bmp.GetRGBA(dst, 1, 1, 1, 1, 4, 0);

    EXPECT_EQ(*reinterpret_cast<const uint32_t*>(dst), (pal[3] & 0xFFFFFFu) | 0x44000000u);
}

TEST(ZBitmapPalOpac, GetBinSizeIncludesPaletteAndOpacity)
{
    const uint8_t indices[16] = {};
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);
    uint8_t opac[16] = {};

    ZBitmapPalOpac bmp;
    MakePalOpacBitmap(bmp, indices, 4, 4, pal, kPalSize, opac);

    // Base: 29 (header + empty name) + 4 (mip size field) + 16 (mip data).
    // Palette: 4 (size field) + 4 * 256 (entries). Opacity: 4 * 4 (1 byte per pixel).
    EXPECT_EQ(bmp.GetBinSize(), 29u + 4u + 16u + 4u + 4u * kPalSize + 16u);
}

TEST(ZBitmapPalOpac, SaveBinWritesOpacityAfterPalette)
{
    const uint8_t indices[4] = { 0, 1, 2, 3 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);
    uint8_t opac[4] = { 0xDE, 0xAD, 0xBE, 0xEF };

    ZBitmapPalOpac bmp;
    MakePalOpacBitmap(bmp, indices, 2, 2, pal, kPalSize, opac);

    std::vector<char> buffer(bmp.GetBinSize(), 0);
    bmp.SaveBin(buffer.data());

    // Base layout: 28-byte header + 1-byte empty name + 4-byte mip size + 4 bytes of mip data.
    constexpr size_t kBaseSize = 28 + 1 + 4 + 4;
    const char* pPalBlock = buffer.data() + kBaseSize;

    EXPECT_EQ(*reinterpret_cast<const int32_t*>(pPalBlock), kPalSize);

    const auto* pSavedPal = reinterpret_cast<const uint32_t*>(pPalBlock + sizeof(int32_t));
    for (int i = 0; i < kPalSize; ++i)
    {
        EXPECT_EQ(pSavedPal[i], pal[i]) << "palette index " << i;
    }

    const auto* pSavedOpac = reinterpret_cast<const uint8_t*>(pPalBlock + sizeof(int32_t) + sizeof(uint32_t) * kPalSize);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(pSavedOpac[i], opac[i]) << "opacity index " << i;
    }
}

TEST(ZBitmapPalOpac, SaveLoadBinRoundTrip)
{
    const uint8_t indices[4] = { 3, 2, 1, 0 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);
    uint8_t opac[4] = { 0x10, 0x20, 0x30, 0x40 };

    ZBitmapPalOpac bmp;
    bmp.SetId(0x55667788);
    MakePalOpacBitmap(bmp, indices, 2, 2, pal, kPalSize, opac);

    std::vector<char> buffer(bmp.GetBinSize(), 0);
    bmp.SaveBin(buffer.data());

    ZBitmapPalOpac loaded;
    loaded.LoadBin(buffer.data());

    EXPECT_EQ(loaded.GetType(), bmp.GetType());
    EXPECT_EQ(loaded.GetId(), 0x55667788u);
    EXPECT_EQ(loaded.GetSizeX(), 2);
    EXPECT_EQ(loaded.GetSizeY(), 2);
    EXPECT_EQ(loaded.GetMipLevelCount(), 1);
    EXPECT_EQ(loaded.GetPalSize(), kPalSize);
    ASSERT_NE(loaded.GetPal(), nullptr);
    ASSERT_NE(loaded.m_pOpac, nullptr);

    for (int i = 0; i < kPalSize; ++i)
    {
        EXPECT_EQ(loaded.GetPal()[i], pal[i]) << "palette index " << i;
    }

    const auto* pLoadedOpac = reinterpret_cast<const uint8_t*>(loaded.m_pOpac);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(pLoadedOpac[i], opac[i]) << "opacity index " << i;
    }

    // Full pipeline: indices + palette + opacity must survive the round trip.
    uint8_t dst[32] = {};
    loaded.GetRGBA(dst, 0, 0, 2, 2, 16, 0);

    const auto* pRow0 = reinterpret_cast<const uint32_t*>(dst);
    const auto* pRow1 = reinterpret_cast<const uint32_t*>(dst + 16);
    EXPECT_EQ(pRow0[0], (pal[3] & 0xFFFFFFu) | 0x10000000u);
    EXPECT_EQ(pRow0[1], (pal[2] & 0xFFFFFFu) | 0x20000000u);
    EXPECT_EQ(pRow1[0], (pal[1] & 0xFFFFFFu) | 0x30000000u);
    EXPECT_EQ(pRow1[1], (pal[0] & 0xFFFFFFu) | 0x40000000u);
}

TEST(ZBitmapPalOpac, GetDataInheritedUsesPaletteOnly)
{
    const uint8_t indices[2] = { 5, 7 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);
    uint8_t opac[2] = { 0x00, 0x00 };

    ZBitmapPalOpac bmp;
    MakePalOpacBitmap(bmp, indices, 2, 1, pal, kPalSize, opac);

    uint32_t out[2] = {};
    ZBitmap::TDataBlock block;
    block.dwSizeX = 2;
    block.dwSizeY = 1;
    block.dwPich = 8;
    block.dwMaskR = 0xFF;
    block.dwMaskG = 0xFF00;
    block.dwMaskB = 0xFF0000;
    block.dwMaskA = 0xFF000000;
    block.dwBitCount = 32;
    block.pData = out;

    // GetData is inherited from ZBitmapPal: palette colors pass through as-is.
    bmp.GetData(&block, 0);

    EXPECT_EQ(out[0], pal[5]);
    EXPECT_EQ(out[1], pal[7]);
}
