#include <Glacier/Render/Bitmap/ZBitmapDXT1.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    constexpr uint32_t kStdMaskR = 0xFF;
    constexpr uint32_t kStdMaskG = 0xFF00;
    constexpr uint32_t kStdMaskB = 0xFF0000;
    constexpr uint32_t kStdMaskA = 0xFF000000;

    // Loads a single raw DXT1 block (8 bytes) as mip level 0.
    void MakeDXT1Bitmap(ZBitmapDXT1& bmp, const uint16_t* block, int sx, int sy)
    {
        bmp.SetSizeX(sx);
        bmp.SetSizeY(sy);
        bmp.SetMIPLevels(1);
        bmp.SetData(const_cast<uint16_t*>(block), 8, sx, sy, 0, true);
    }
}

TEST(ZBitmapDXT1, CtorSetsTypeAndDefaults)
{
    ZBitmapDXT1 bmp;

    EXPECT_EQ(bmp.GetType(), 'DXT1');
    EXPECT_EQ(bmp.GetSizeX(), 0);
    EXPECT_EQ(bmp.GetSizeY(), 0);
    EXPECT_EQ(bmp.GetMipLevelCount(), 0);
    EXPECT_FLOAT_EQ(bmp.GetScaleFactor(), 1.0f);
}

TEST(ZBitmapDXT1, GetRGBADecodesFourColorBlock)
{
    // c0 (red) > c1 (green): 4-color mode. Indices: px0..px3 = 0,1,2,3, rest 0.
    const uint16_t block[4] = { 0xF800, 0x07E0, 0x00E4, 0x0000 };
    ZBitmapDXT1 bmp;
    MakeDXT1Bitmap(bmp, block, 4, 4);

    uint32_t dst[16] = {};
    bmp.GetRGBA(reinterpret_cast<uint8_t*>(dst), 0, 0, 4, 4, 16, 0);

    EXPECT_EQ(dst[0], 0xFFF80000u); // palette[0]: R=0xF8
    EXPECT_EQ(dst[1], 0xFF00FC00u); // palette[1]: G=0xFC
    EXPECT_EQ(dst[2], 0xFFA55400u); // palette[2]: (2c0+c1)/3
    EXPECT_EQ(dst[3], 0xFF52A800u); // palette[3]: (c0+2c1)/3
    EXPECT_EQ(dst[4], 0xFFF80000u); // remaining pixels use index 0
}

TEST(ZBitmapDXT1, GetRGBADecodesThreeColorBlock)
{
    // c0 (black) <= c1 (white): 3-color mode with transparent index 3.
    const uint16_t block[4] = { 0x0000, 0xFFFF, 0x00E4, 0x0000 };
    ZBitmapDXT1 bmp;
    MakeDXT1Bitmap(bmp, block, 4, 4);

    uint32_t dst[16] = {};
    bmp.GetRGBA(reinterpret_cast<uint8_t*>(dst), 0, 0, 4, 4, 16, 0);

    EXPECT_EQ(dst[0], 0xFF000000u); // palette[0]: black
    EXPECT_EQ(dst[1], 0xFFF8FCF8u); // palette[1]: white
    EXPECT_EQ(dst[2], 0xFF7C7E7Cu); // palette[2]: (c0+c1)/2
    EXPECT_EQ(dst[3], 0x00000000u); // palette[3]: transparent
}

TEST(ZBitmapDXT1, GetRGBAIgnoresSubRectAndPitch)
{
    // PC behavior: GetRGBA always decodes the whole mip level into pDst.
    const uint16_t block[4] = { 0xF800, 0x07E0, 0x00E4, 0x0000 };
    ZBitmapDXT1 bmp;
    MakeDXT1Bitmap(bmp, block, 4, 4);

    uint32_t dst[16] = {};
    bmp.GetRGBA(reinterpret_cast<uint8_t*>(dst), 2, 2, 1, 1, 4, 0);

    EXPECT_EQ(dst[0], 0xFFF80000u);
    EXPECT_EQ(dst[1], 0xFF00FC00u);
    EXPECT_EQ(dst[15], 0xFFF80000u);
}

TEST(ZBitmapDXT1, GetDataConvertsBGRAToRGBA)
{
    const uint16_t block[4] = { 0xF800, 0x07E0, 0x0000, 0x0000 };
    ZBitmapDXT1 bmp;
    MakeDXT1Bitmap(bmp, block, 4, 4);

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

    // Decoded 0xFFF80000 (R at bits 16-23) becomes RGBA 0xFF0000F8.
    EXPECT_EQ(out[0], 0xFF0000F8u);
}

TEST(ZBitmapDXT1, PackMipLevelSolidColor)
{
    uint32_t pixels[16];
    for (int i = 0; i < 16; ++i)
    {
        pixels[i] = 0xFF0000FF; // R=255, G=0, B=0, A=255
    }

    ZBitmapDXT1 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 4, 4, pixels);

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_EQ(pMip->dwSize, 8u);

    // Solid red: both endpoints collapse to 0xF800, all indices are 0.
    const auto* pData = static_cast<const uint8_t*>(pMip->pData);
    const uint8_t expected[8] = { 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00 };
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(pData[i], expected[i]) << "byte " << i;
    }

    // Round trip: decodes back to red.
    uint32_t dst[16] = {};
    bmp.GetRGBA(reinterpret_cast<uint8_t*>(dst), 0, 0, 4, 4, 16, 0);
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(dst[i], 0xFFF80000u) << "pixel " << i;
    }
}

TEST(ZBitmapDXT1, PackMipLevelTransparentPixel)
{
    uint32_t pixels[16];
    for (int i = 0; i < 16; ++i)
    {
        pixels[i] = 0xFF0000FF; // opaque red
    }
    pixels[0] = 0x00000000; // transparent

    ZBitmapDXT1 bmp;
    bmp.SetSizeX(4);
    bmp.SetSizeY(4);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 4, 4, pixels);

    // Alpha mode: c0=0x0000 <= c1=0xF800, transparent pixel gets index 3,
    // opaque pixels get index 1.
    const auto* pData = static_cast<const uint8_t*>(bmp.GetMIPLevel(0)->pData);
    const uint8_t expected[8] = { 0x00, 0x00, 0x00, 0xF8, 0x57, 0x55, 0x55, 0x55 };
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(pData[i], expected[i]) << "byte " << i;
    }

    uint32_t dst[16] = {};
    bmp.GetRGBA(reinterpret_cast<uint8_t*>(dst), 0, 0, 4, 4, 16, 0);
    EXPECT_EQ(dst[0], 0x00000000u); // transparent
    for (int i = 1; i < 16; ++i)
    {
        EXPECT_EQ(dst[i], 0xFFF80000u) << "pixel " << i;
    }
}

TEST(ZBitmapDXT1, PackMipLevelClampsEdgeBlocks)
{
    // 2x2 source: the 4x4 block gather clamps to the last row/column.
    uint32_t pixels[4];
    for (int i = 0; i < 4; ++i)
    {
        pixels[i] = 0xFF0000FF; // opaque red
    }

    ZBitmapDXT1 bmp;
    bmp.SetSizeX(2);
    bmp.SetSizeY(2);
    bmp.SetMIPLevels(1);
    bmp.PackMipLevel(0, 2, 2, pixels);

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_EQ(pMip->dwSize, 8u);
    EXPECT_EQ(pMip->dwSizeX, 2);
    EXPECT_EQ(pMip->dwSizeY, 2);

    // Clamped gather produced a solid red block.
    const auto* pData = static_cast<const uint8_t*>(pMip->pData);
    const uint8_t expected[8] = { 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00 };
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(pData[i], expected[i]) << "byte " << i;
    }
}
