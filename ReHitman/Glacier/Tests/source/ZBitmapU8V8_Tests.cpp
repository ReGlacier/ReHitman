#include <Glacier/Render/Bitmap/ZBitmapU8V8.h>
#include <gtest/gtest.h>

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

    // Builds a U8V8 bitmap from U/V pairs (through PackMipLevel).
    void MakeU8V8Bitmap(ZBitmapU8V8& bmp, const uint16_t* values, int sx, int sy)
    {
        const int count = sx * sy;
        uint32_t rgba[256] = {};
        for (int i = 0; i < count && i < 256; ++i)
        {
            // U is sourced from the G channel (bits 8-15), V from the A channel (bits 24-31).
            const uint32_t u = values[i] & 0xFF;
            const uint32_t v = (values[i] >> 8) & 0xFF;
            rgba[i] = (u << 8) | (v << 24);
        }

        bmp.SetSizeX(sx);
        bmp.SetSizeY(sy);
        bmp.SetMIPLevels(1);
        bmp.PackMipLevel(0, sx, sy, rgba);
    }
}

TEST(ZBitmapU8V8, CtorSetsTypeAndDefaults)
{
    ZBitmapU8V8 bmp;

    EXPECT_EQ(bmp.GetType(), 'U8V8');
    EXPECT_EQ(bmp.GetSizeX(), 0);
    EXPECT_EQ(bmp.GetSizeY(), 0);
    EXPECT_EQ(bmp.GetMipLevelCount(), 0);
    EXPECT_FLOAT_EQ(bmp.GetScaleFactor(), 1.0f);
}

TEST(ZBitmapU8V8, PackMipLevelPacksGreenAndAlphaChannels)
{
    uint32_t pixels[4] = { 0x00000000, 0x0000FF00, 0x12345678, 0xFF000000 };
    ZBitmapU8V8 bmp;
    bmp.SetSizeX(2);
    bmp.SetSizeY(2);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 2, 2, pixels);

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_TRUE(pMip->bAllocated);
    EXPECT_EQ(pMip->dwSize, 8u); // 2 bytes per pixel
    EXPECT_EQ(pMip->dwSizeX, 2);
    EXPECT_EQ(pMip->dwSizeY, 2);

    const auto* pData = static_cast<const uint16_t*>(pMip->pData);
    EXPECT_EQ(pData[0], 0x0000);
    EXPECT_EQ(pData[1], 0x00FF); // U=0xFF, V=0x00
    EXPECT_EQ(pData[2], 0x1256); // U=0x56 (G), V=0x12 (A)
    EXPECT_EQ(pData[3], 0xFF00); // U=0x00, V=0xFF
}

TEST(ZBitmapU8V8, GetRGBAExpandsToOpaqueTexel)
{
    const uint16_t values[4] = { 0x0000, 0x0040, 0x8000, 0xFFFF };
    ZBitmapU8V8 bmp;
    MakeU8V8Bitmap(bmp, values, 2, 2);

    uint8_t dst[32] = {};
    bmp.GetRGBA(dst, 0, 0, 2, 2, 16, 0);

    const auto* pRow0 = reinterpret_cast<const uint32_t*>(dst);
    const auto* pRow1 = reinterpret_cast<const uint32_t*>(dst + 16);
    EXPECT_EQ(pRow0[0], 0xFF0000FFu); // R=255, G=0,    B=0,    A=255
    EXPECT_EQ(pRow0[1], 0xFF0040FFu); // R=255, G=0x40, B=0,    A=255
    EXPECT_EQ(pRow1[0], 0xFF8000FFu); // R=255, G=0,    B=0x80, A=255
    EXPECT_EQ(pRow1[1], 0xFFFFFFFFu); // R=255, G=0xFF, B=0xFF, A=255
}

TEST(ZBitmapU8V8, GetRGBAReadsSubRect)
{
    const uint16_t values[4] = { 0x1111, 0x2222, 0x3333, 0x4444 };
    ZBitmapU8V8 bmp;
    MakeU8V8Bitmap(bmp, values, 2, 2);

    uint8_t dst[4] = {};
    bmp.GetRGBA(dst, 1, 1, 1, 1, 4, 0);

    EXPECT_EQ(*reinterpret_cast<const uint32_t*>(dst), 0xFF4444FFu);
}

TEST(ZBitmapU8V8, GetDataMapsUToRedAndVToGreen)
{
    const uint16_t values[2] = { 0x8040, 0x0000 };
    ZBitmapU8V8 bmp;
    MakeU8V8Bitmap(bmp, values, 2, 1);

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

    // The raw texel feeds the converter: U -> R, V -> G, B and A left zero.
    EXPECT_EQ(out[0], 0x00008040u);
    EXPECT_EQ(out[1], 0x00000000u);
}

TEST(ZBitmapU8V8, GetDataConvertsTo16Bit)
{
    const uint16_t values[2] = { 0x00FF, 0xFF00 };
    ZBitmapU8V8 bmp;
    MakeU8V8Bitmap(bmp, values, 2, 1);

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

    EXPECT_EQ(out[0], 0x1F);   // only U survives in the red channel
    EXPECT_EQ(out[1], 0x3E0);  // only V survives in the green channel
}

TEST(ZBitmapU8V8, SwizzleMortonOrder4x4)
{
    uint16_t values[16] = {};
    for (uint16_t i = 0; i < 16; ++i)
    {
        values[i] = i;
    }

    ZBitmapU8V8 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.SetData(values, 32, 4, 4, 0, true);

    bmp.Swizzle();

    // Morton (Z-order) permutation of a 4x4 bitmap.
    constexpr uint16_t kExpected[16] = { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };
    const auto* pData = static_cast<const uint16_t*>(bmp.GetMIPLevel(0)->pData);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], kExpected[i]) << "index " << i;
    }
}

TEST(ZBitmapU8V8, SwizzleTwiceRestoresOrder)
{
    uint16_t values[16] = {};
    for (uint16_t i = 0; i < 16; ++i)
    {
        values[i] = i * 7;
    }

    ZBitmapU8V8 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.SetData(values, 32, 4, 4, 0, true);

    bmp.Swizzle();
    bmp.Swizzle();

    const auto* pData = static_cast<const uint16_t*>(bmp.GetMIPLevel(0)->pData);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], values[i]) << "index " << i;
    }
}
