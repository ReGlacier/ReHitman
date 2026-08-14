#include <Glacier/Render/Bitmap/ZBitmapPal.h>
#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

using namespace Glacier;

namespace
{
    constexpr uint32_t kStdMaskR = 0xFF;
    constexpr uint32_t kStdMaskG = 0xFF00;
    constexpr uint32_t kStdMaskB = 0xFF0000;
    constexpr uint32_t kStdMaskA = 0xFF000000;

    constexpr uint32_t k16MaskR = 0x1F;
    constexpr uint32_t k16MaskG = 0x3E0;
    constexpr uint32_t k16MaskB = 0x7C00;
    constexpr uint32_t k16MaskA = 0x8000;

    constexpr int kPalSize = 256;

    // Fills a palette with distinct easily recognizable colors.
    void MakePalette(uint32_t* pPal, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            pPal[i] = 0xFF000000u | (static_cast<uint32_t>(i) << 16) | (static_cast<uint32_t>(i) << 8) | static_cast<uint32_t>(i);
        }
    }

    // Builds a paletted bitmap with the given 8-bit indices (through SetData).
    void MakePalBitmap(ZBitmapPal& bmp, const uint8_t* indices, int sx, int sy, uint32_t* pPal, int palSize, bool bCopyPal = true)
    {
        bmp.SetSizeX(sx);
        bmp.SetSizeY(sy);
        bmp.SetMIPLevels(1);
        bmp.SetData(const_cast<uint8_t*>(indices), sx * sy, sx, sy, 0, true);
        bmp.SetPalette(pPal, palSize, bCopyPal);
    }
}

TEST(ZBitmapPal, CtorSetsTypeAndDefaults)
{
    ZBitmapPal bmp;

    EXPECT_EQ(bmp.GetType(), 1346456654); // 'PALN'
    EXPECT_EQ(bmp.GetSizeX(), 0);
    EXPECT_EQ(bmp.GetSizeY(), 0);
    EXPECT_EQ(bmp.GetMipLevelCount(), 0);
    EXPECT_FLOAT_EQ(bmp.GetScaleFactor(), 1.0f);
    EXPECT_EQ(bmp.GetPal(), nullptr);
    EXPECT_EQ(bmp.GetPalSize(), 0);
}

TEST(ZBitmapPal, SetPaletteCopiesWhenRequested)
{
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    bmp.SetPalette(pal, kPalSize, true);

    EXPECT_EQ(bmp.GetPalSize(), kPalSize);
    ASSERT_NE(bmp.GetPal(), nullptr);
    EXPECT_NE(bmp.GetPal(), pal); // owned copy, not the source pointer

    for (int i = 0; i < kPalSize; ++i)
    {
        EXPECT_EQ(bmp.GetPal()[i], pal[i]) << "index " << i;
    }

    // Later mutations of the source must not leak into the owned copy.
    pal[0] = 0xDEADBEEF;
    EXPECT_NE(bmp.GetPal()[0], 0xDEADBEEFu);
}

TEST(ZBitmapPal, SetPaletteAliasesWithoutCopy)
{
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    bmp.SetPalette(pal, kPalSize, false);

    EXPECT_EQ(bmp.GetPalSize(), kPalSize);
    EXPECT_EQ(bmp.GetPal(), pal); // aliases the caller buffer
}

TEST(ZBitmapPal, SetPaletteReplacesOwnedPalette)
{
    uint32_t pal1[kPalSize] = {};
    uint32_t pal2[4] = { 0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    MakePalette(pal1, kPalSize);

    ZBitmapPal bmp;
    bmp.SetPalette(pal1, kPalSize, true);
    bmp.SetPalette(pal2, 4, true);

    EXPECT_EQ(bmp.GetPalSize(), 4);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(bmp.GetPal()[i], pal2[i]) << "index " << i;
    }
}

TEST(ZBitmapPal, GetRGBAMapsIndicesThroughPalette)
{
    const uint8_t indices[4] = { 0, 1, 2, 3 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 2, 2, pal, kPalSize);

    uint8_t dst[32] = {};
    bmp.GetRGBA(dst, 0, 0, 2, 2, 16, 0);

    const auto* pRow0 = reinterpret_cast<const uint32_t*>(dst);
    const auto* pRow1 = reinterpret_cast<const uint32_t*>(dst + 16);
    EXPECT_EQ(pRow0[0], pal[0]);
    EXPECT_EQ(pRow0[1], pal[1]);
    EXPECT_EQ(pRow1[0], pal[2]);
    EXPECT_EQ(pRow1[1], pal[3]);
}

TEST(ZBitmapPal, GetRGBAAssertsOnPartialRead)
{
    const uint8_t indices[4] = { 0, 1, 2, 3 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 2, 2, pal, kPalSize);

    uint8_t dst[32] = {};
    // PC asserts that the requested region matches the full mip level.
    EXPECT_THROW(bmp.GetRGBA(dst, 0, 0, 1, 2, 16, 0), std::runtime_error);
    EXPECT_THROW(bmp.GetRGBA(dst, 0, 0, 2, 1, 16, 0), std::runtime_error);
}

TEST(ZBitmapPal, GetDataWritesPaletteColors32)
{
    const uint8_t indices[2] = { 5, 7 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 2, 1, pal, kPalSize);

    uint32_t out[2] = {};
    ZBitmap::TDataBlock block;
    block.dwSizeX = 2;
    block.dwSizeY = 1;
    block.dwPich = 8;
    block.dwMaskR = kStdMaskR;
    block.dwMaskG = kStdMaskG;
    block.dwMaskB = kStdMaskB;
    block.dwMaskA = kStdMaskA;
    block.dwBitCount = 32;
    block.pData = out;

    bmp.GetData(&block, 0);

    EXPECT_EQ(out[0], pal[5]);
    EXPECT_EQ(out[1], pal[7]);
}

TEST(ZBitmapPal, GetDataConvertsTo16Bit)
{
    const uint8_t indices[2] = { 0, 1 };
    uint32_t pal[kPalSize] = {};
    pal[0] = 0xFFFFFFFF;
    pal[1] = 0xFF000000; // opaque black

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 2, 1, pal, kPalSize);

    uint16_t out[2] = {};
    ZBitmap::TDataBlock block;
    block.dwSizeX = 2;
    block.dwSizeY = 1;
    block.dwPich = 4;
    block.dwMaskR = k16MaskR;
    block.dwMaskG = k16MaskG;
    block.dwMaskB = k16MaskB;
    block.dwMaskA = k16MaskA;
    block.dwBitCount = 16;
    block.pData = out;

    bmp.GetData(&block, 0);

    EXPECT_EQ(out[0], 0xFFFF);
    EXPECT_EQ(out[1], 0x8000); // only the alpha bit survives
}

TEST(ZBitmapPal, GetDataRespectsOrigin)
{
    const uint8_t indices[4] = { 0, 1, 2, 3 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 2, 2, pal, kPalSize);

    uint32_t out[1] = {};
    ZBitmap::TDataBlock block;
    block.dwPosX = 1;
    block.dwPosY = 1;
    block.dwSizeX = 1;
    block.dwSizeY = 1;
    block.dwPich = 4;
    block.dwMaskR = kStdMaskR;
    block.dwMaskG = kStdMaskG;
    block.dwMaskB = kStdMaskB;
    block.dwMaskA = kStdMaskA;
    block.dwBitCount = 32;
    block.pData = out;

    bmp.GetData(&block, 0);

    EXPECT_EQ(out[0], pal[3]);
}

TEST(ZBitmapPal, GetBinSizeIncludesPalette)
{
    const uint8_t indices[16] = {};
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 4, 4, pal, kPalSize);

    // Base: 29 (header + empty name) + 4 (mip size field) + 16 (mip data).
    // Palette: 4 (size field) + 4 * 256 (entries).
    EXPECT_EQ(bmp.GetBinSize(), 29u + 4u + 16u + 4u + 4u * kPalSize);
}

TEST(ZBitmapPal, SaveBinWritesPaletteAfterBaseData)
{
    const uint8_t indices[4] = { 0, 1, 2, 3 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 2, 2, pal, kPalSize);

    std::vector<char> buffer(bmp.GetBinSize(), 0);
    bmp.SaveBin(buffer.data());

    // Base layout: 28-byte header + 1-byte empty name + 4-byte mip size + 4 bytes of mip data.
    constexpr size_t kBaseSize = 28 + 1 + 4 + 4;
    const char* pPalBlock = buffer.data() + kBaseSize;

    EXPECT_EQ(*reinterpret_cast<const int32_t*>(pPalBlock), kPalSize);

    const auto* pSavedPal = reinterpret_cast<const uint32_t*>(pPalBlock + sizeof(int32_t));
    for (int i = 0; i < kPalSize; ++i)
    {
        EXPECT_EQ(pSavedPal[i], pal[i]) << "index " << i;
    }
}

TEST(ZBitmapPal, SaveLoadBinRoundTrip)
{
    const uint8_t indices[4] = { 3, 2, 1, 0 };
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    bmp.SetId(0x11223344);
    MakePalBitmap(bmp, indices, 2, 2, pal, kPalSize);

    std::vector<char> buffer(bmp.GetBinSize(), 0);
    bmp.SaveBin(buffer.data());

    ZBitmapPal loaded;
    loaded.LoadBin(buffer.data());

    EXPECT_EQ(loaded.GetType(), bmp.GetType());
    EXPECT_EQ(loaded.GetId(), 0x11223344u);
    EXPECT_EQ(loaded.GetSizeX(), 2);
    EXPECT_EQ(loaded.GetSizeY(), 2);
    EXPECT_EQ(loaded.GetMipLevelCount(), 1);
    EXPECT_EQ(loaded.GetPalSize(), kPalSize);
    ASSERT_NE(loaded.GetPal(), nullptr);

    for (int i = 0; i < kPalSize; ++i)
    {
        EXPECT_EQ(loaded.GetPal()[i], pal[i]) << "palette index " << i;
    }

    // Palette indices must survive the round trip too.
    uint8_t dst[32] = {};
    loaded.GetRGBA(dst, 0, 0, 2, 2, 16, 0);

    const auto* pRow0 = reinterpret_cast<const uint32_t*>(dst);
    const auto* pRow1 = reinterpret_cast<const uint32_t*>(dst + 16);
    EXPECT_EQ(pRow0[0], pal[3]);
    EXPECT_EQ(pRow0[1], pal[2]);
    EXPECT_EQ(pRow1[0], pal[1]);
    EXPECT_EQ(pRow1[1], pal[0]);
}

TEST(ZBitmapPal, SwizzleMortonOrder4x4)
{
    uint8_t indices[16] = {};
    for (uint8_t i = 0; i < 16; ++i)
    {
        indices[i] = i;
    }
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 4, 4, pal, kPalSize);

    bmp.Swizzle();

    // Morton (Z-order) permutation of a 4x4 bitmap.
    constexpr uint8_t kExpected[16] = { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };
    const auto* pData = static_cast<const uint8_t*>(bmp.GetMIPLevel(0)->pData);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], kExpected[i]) << "index " << i;
    }
}

TEST(ZBitmapPal, SwizzleTwiceRestoresOrder)
{
    uint8_t indices[16] = {};
    for (uint8_t i = 0; i < 16; ++i)
    {
        indices[i] = i * 7;
    }
    uint32_t pal[kPalSize] = {};
    MakePalette(pal, kPalSize);

    ZBitmapPal bmp;
    MakePalBitmap(bmp, indices, 4, 4, pal, kPalSize);

    bmp.Swizzle();
    bmp.Swizzle();

    const auto* pData = static_cast<const uint8_t*>(bmp.GetMIPLevel(0)->pData);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], indices[i]) << "index " << i;
    }
}

TEST(ZBitmapPal, PackMipLevelDoesNothing)
{
    ZBitmapPal bmp;
    bmp.SetSizeX(2);
    bmp.SetSizeY(2);
    bmp.SetMIPLevels(1);

    uint32_t pixels[4] = { 0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    bmp.PackMipLevel(0, 2, 2, pixels);

    const ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_FALSE(pMip->bAllocated);
    EXPECT_EQ(pMip->pData, nullptr);
    EXPECT_EQ(pMip->dwSize, 0u);
}

TEST(ZBitmapPal, CreateAsserts)
{
    ZBitmapPal bmp;
    ZBitmapPal other;

    // PC routes Create to a debug break stub; palettes are assigned via SetPalette instead.
    EXPECT_THROW(bmp.Create(other, false), std::runtime_error);
}
