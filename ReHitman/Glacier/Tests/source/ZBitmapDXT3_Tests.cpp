#include <Glacier/Render/Bitmap/ZBitmapDXT3.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    constexpr uint32_t kStdMaskR = 0xFF;
    constexpr uint32_t kStdMaskG = 0xFF00;
    constexpr uint32_t kStdMaskB = 0xFF0000;
    constexpr uint32_t kStdMaskA = 0xFF000000;

    // Loads a single raw DXT3 block (16 bytes) as mip level 0.
    void MakeDXT3Bitmap(ZBitmapDXT3& bmp, const uint16_t* block, int sx, int sy)
    {
        bmp.SetSizeX(sx);
        bmp.SetSizeY(sy);
        bmp.SetMIPLevels(1);
        bmp.SetData(const_cast<uint16_t*>(block), 16, sx, sy, 0, true);
    }
}

TEST(ZBitmapDXT3, CtorSetsTypeAndDefaults)
{
    ZBitmapDXT3 bmp;

    EXPECT_EQ(bmp.GetType(), 'DXT3');
    EXPECT_EQ(bmp.GetSizeX(), 0);
    EXPECT_EQ(bmp.GetSizeY(), 0);
    EXPECT_EQ(bmp.GetMipLevelCount(), 0);
    EXPECT_FLOAT_EQ(bmp.GetScaleFactor(), 1.0f);
}

TEST(ZBitmapDXT3, DecodeMipLevelDecodesColor)
{
    // Alpha fully opaque, color: c0 (red) > c1 (green), all indices 0.
    const uint16_t block[8] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0x07E0, 0x0000, 0x0000 };
    ZBitmapDXT3 bmp;
    MakeDXT3Bitmap(bmp, block, 4, 4);

    uint32_t dst[16] = {};
    bmp.DecodeMipLevel(reinterpret_cast<int*>(dst), 0);

    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(dst[i], 0xFFF80000u) << "pixel " << i;
    }
}

TEST(ZBitmapDXT3, DecodeMipLevelExpandsAlphaNibbles)
{
    // Alpha nibble per pixel = pixel index; color is solid red, all indices 0.
    const uint16_t block[8] = { 0x3210, 0x7654, 0xBA98, 0xFEDC, 0xF800, 0x07E0, 0x0000, 0x0000 };
    ZBitmapDXT3 bmp;
    MakeDXT3Bitmap(bmp, block, 4, 4);

    uint32_t dst[16] = {};
    bmp.DecodeMipLevel(reinterpret_cast<int*>(dst), 0);

    for (int i = 0; i < 16; ++i)
    {
        // 4-bit alpha i expands to i * 0x11.
        EXPECT_EQ(dst[i], (static_cast<uint32_t>(i * 0x11) << 24) | 0x00F80000u) << "pixel " << i;
    }
}

TEST(ZBitmapDXT3, GetDataConvertsBGRAToRGBA)
{
    // Alpha 0x8 everywhere (A=0x88), solid red.
    const uint16_t block[8] = { 0x8888, 0x8888, 0x8888, 0x8888, 0xF800, 0x07E0, 0x0000, 0x0000 };
    ZBitmapDXT3 bmp;
    MakeDXT3Bitmap(bmp, block, 4, 4);

    uint32_t out[16] = {};
    ZBitmap::TDataBlock data;
    data.dwSizeX = 4;
    data.dwSizeY = 4;
    data.dwPich = 16;
    data.dwMaskR = kStdMaskR;
    data.dwMaskG = kStdMaskG;
    data.dwMaskB = kStdMaskB;
    data.dwMaskA = kStdMaskA;
    data.dwBitCount = 32;
    data.pData = out;

    bmp.GetData(&data, 0);

    // Decoded 0x88F80000 (R at bits 16-23) becomes RGBA 0x880000F8.
    EXPECT_EQ(out[0], 0x880000F8u);
}

TEST(ZBitmapDXT3, PackMipLevelSolidColor)
{
    uint32_t pixels[16];
    for (int i = 0; i < 16; ++i)
    {
        pixels[i] = 0xFFFF0000; // R=0, G=0, B=255, A=255
    }

    ZBitmapDXT3 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 4, 4, pixels);

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_EQ(pMip->dwSize, 16u);

    // Alpha nibbles all 0xF; solid blue: both endpoints collapse to 0x001F, indices 0.
    const auto* pData = static_cast<const uint8_t*>(pMip->pData);
    const uint8_t expected[16] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], expected[i]) << "byte " << i;
    }

    // Round trip: decodes back to opaque blue.
    uint32_t dst[16] = {};
    bmp.DecodeMipLevel(reinterpret_cast<int*>(dst), 0);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(dst[i], 0xFF0000F8u) << "pixel " << i;
    }
}

TEST(ZBitmapDXT3, PackMipLevelAlphaGradient)
{
    // Per-pixel alpha i * 0x11 over solid red; DXT3 alpha survives exactly.
    uint32_t pixels[16];
    for (int i = 0; i < 16; ++i)
    {
        pixels[i] = 0x000000FF | (static_cast<uint32_t>(i * 0x11) << 24);
    }

    ZBitmapDXT3 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 4, 4, pixels);

    uint32_t dst[16] = {};
    bmp.DecodeMipLevel(reinterpret_cast<int*>(dst), 0);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(dst[i], (static_cast<uint32_t>(i * 0x11) << 24) | 0x00F80000u) << "pixel " << i;
    }
}

TEST(ZBitmapDXT3, PackMipLevelClampsEdgeBlocks)
{
    // 2x2 source: the 4x4 block gather clamps to the last row/column.
    uint32_t pixels[4];
    for (int i = 0; i < 4; ++i)
    {
        pixels[i] = 0xFFFF0000; // opaque blue
    }

    ZBitmapDXT3 bmp;
    bmp.SetSizeX(2);
    bmp.SetSizeY(2);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 2, 2, pixels);

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_EQ(pMip->dwSize, 16u);
    EXPECT_EQ(pMip->dwSizeX, 2);
    EXPECT_EQ(pMip->dwSizeY, 2);

    const auto* pData = static_cast<const uint8_t*>(pMip->pData);
    const uint8_t expected[16] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(pData[i], expected[i]) << "byte " << i;
    }
}
