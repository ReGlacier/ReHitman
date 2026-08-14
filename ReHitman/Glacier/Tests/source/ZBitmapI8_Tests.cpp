#include <Glacier/Render/Bitmap/ZBitmapI8.h>
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

    // Builds an I8 bitmap with the given intensity values (through PackMipLevel).
    void MakeI8Bitmap(ZBitmapI8& bmp, const uint8_t* values, int sx, int sy)
    {
        const int count = sx * sy;
        uint32_t rgba[256] = {};
        for (int i = 0; i < count && i < 256; ++i)
        {
            // Intensity is sourced from the B channel (bits 16-23).
            rgba[i] = static_cast<uint32_t>(values[i]) << 16;
        }

        bmp.SetSizeX(sx);
        bmp.SetSizeY(sy);
        bmp.SetMIPLevels(1);
        bmp.PackMipLevel(0, sx, sy, rgba);
    }
}

TEST(ZBitmapI8, CtorSetsTypeAndDefaults)
{
    ZBitmapI8 bmp;

    EXPECT_EQ(bmp.GetType(), 1228415008); // 'I8  '
    EXPECT_EQ(bmp.GetSizeX(), 0);
    EXPECT_EQ(bmp.GetSizeY(), 0);
    EXPECT_EQ(bmp.GetMipLevelCount(), 0);
    EXPECT_FLOAT_EQ(bmp.GetScaleFactor(), 1.0f);
}

TEST(ZBitmapI8, PackMipLevelExtractsBlueChannel)
{
    uint32_t pixels[4] = { 0x00000000, 0x00FF0000, 0x12345678, 0xFF000000 };
    ZBitmapI8 bmp;
    bmp.SetSizeX(2);
    bmp.SetSizeY(2);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 2, 2, pixels);

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_TRUE(pMip->bAllocated);
    EXPECT_EQ(pMip->dwSize, 4u); // 1 byte per pixel
    EXPECT_EQ(pMip->dwSizeX, 2);
    EXPECT_EQ(pMip->dwSizeY, 2);

    const auto* pData = static_cast<const uint8_t*>(pMip->pData);
    EXPECT_EQ(pData[0], 0x00);
    EXPECT_EQ(pData[1], 0xFF);
    EXPECT_EQ(pData[2], 0x34);
    EXPECT_EQ(pData[3], 0x00);
}

TEST(ZBitmapI8, GetRGBAExpandsGrayscale)
{
    const uint8_t values[4] = { 0x00, 0x40, 0x80, 0xFF };
    ZBitmapI8 bmp;
    MakeI8Bitmap(bmp, values, 2, 2);

    uint8_t dst[32] = {};
    bmp.GetRGBA(dst, 0, 0, 2, 2, 16, 0);

    const auto* pRow0 = reinterpret_cast<const uint32_t*>(dst);
    const auto* pRow1 = reinterpret_cast<const uint32_t*>(dst + 16);
    EXPECT_EQ(pRow0[0], 0x00000000u);
    EXPECT_EQ(pRow0[1], 0x40404040u);
    EXPECT_EQ(pRow1[0], 0x80808080u);
    EXPECT_EQ(pRow1[1], 0xFFFFFFFFu);
}

TEST(ZBitmapI8, GetRGBAReadsSubRect)
{
    const uint8_t values[4] = { 0x11, 0x22, 0x33, 0x44 };
    ZBitmapI8 bmp;
    MakeI8Bitmap(bmp, values, 2, 2);

    uint8_t dst[4] = {};
    bmp.GetRGBA(dst, 1, 1, 1, 1, 4, 0);

    EXPECT_EQ(*reinterpret_cast<const uint32_t*>(dst), 0x44444444u);
}

TEST(ZBitmapI8, GetDataYieldsOpaqueGrayscale)
{
    const uint8_t values[2] = { 0x80, 0x00 };
    ZBitmapI8 bmp;
    MakeI8Bitmap(bmp, values, 2, 1);

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

    // Unlike GetRGBA, GetData produces opaque pixels: R=G=B=v, A=255.
    EXPECT_EQ(out[0], 0xFF808080u);
    EXPECT_EQ(out[1], 0xFF000000u);
}

TEST(ZBitmapI8, GetDataConvertsTo16Bit)
{
    const uint8_t values[2] = { 0xFF, 0x00 };
    ZBitmapI8 bmp;
    MakeI8Bitmap(bmp, values, 2, 1);

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

TEST(ZBitmapI8, SwizzleMortonOrder4x4)
{
    uint8_t values[16] = {};
    for (uint8_t i = 0; i < 16; ++i)
    {
        values[i] = i;
    }

    ZBitmapI8 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.SetData(values, 16, 4, 4, 0, true);

    bmp.Swizzle();

    // Morton (Z-order) permutation of a 4x4 bitmap.
    constexpr uint8_t kExpected[16] = { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };
    const auto* pData = static_cast<const uint8_t*>(bmp.GetMIPLevel(0)->pData);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], kExpected[i]) << "index " << i;
    }
}

TEST(ZBitmapI8, SwizzleTwiceRestoresOrder)
{
    uint8_t values[16] = {};
    for (uint8_t i = 0; i < 16; ++i)
    {
        values[i] = i * 7;
    }

    ZBitmapI8 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.SetData(values, 16, 4, 4, 0, true);

    bmp.Swizzle();
    bmp.Swizzle();

    const auto* pData = static_cast<const uint8_t*>(bmp.GetMIPLevel(0)->pData);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], values[i]) << "index " << i;
    }
}
