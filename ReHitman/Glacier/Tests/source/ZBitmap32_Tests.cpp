#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    // Internal RGBA32 layout: R at bits 0-7, G at 8-15, B at 16-23, A at 24-31.
    constexpr uint32_t kStdMaskR = 0xFF;
    constexpr uint32_t kStdMaskG = 0xFF00;
    constexpr uint32_t kStdMaskB = 0xFF0000;
    constexpr uint32_t kStdMaskA = 0xFF000000;

    // A1R5G5B5-style reduced masks.
    constexpr uint32_t k16MaskR = 0x1F;
    constexpr uint32_t k16MaskG = 0x3E0;
    constexpr uint32_t k16MaskB = 0x7C00;
    constexpr uint32_t k16MaskA = 0x8000;

    ZColorMask MakeStdToStdMask()
    {
        ZColorMask mask;
        mask.m_Src.dwMask[0] = kStdMaskR;
        mask.m_Src.dwMask[1] = kStdMaskG;
        mask.m_Src.dwMask[2] = kStdMaskB;
        mask.m_Src.dwMask[3] = kStdMaskA;
        mask.InitShifters(mask.m_Src);
        mask.m_Dst = mask.m_Src;
        return mask;
    }
}

TEST(ZColorMask, InitShiftersStandardMasks)
{
    ZColorMask mask;
    mask.m_Src.dwMask[0] = kStdMaskR;
    mask.m_Src.dwMask[1] = kStdMaskG;
    mask.m_Src.dwMask[2] = kStdMaskB;
    mask.m_Src.dwMask[3] = kStdMaskA;

    mask.InitShifters(mask.m_Src);

    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(mask.m_Src.dwPos[i], 8u * i);
        EXPECT_EQ(mask.m_Src.dwSize[i], 0u); // 8-bit channel: normalization shift is 8 - 8 = 0
    }
}

TEST(ZColorMask, InitShiftersReducedMasks)
{
    ZColorMask mask;
    mask.m_Src.dwMask[0] = k16MaskR;
    mask.m_Src.dwMask[1] = k16MaskG;
    mask.m_Src.dwMask[2] = k16MaskB;
    mask.m_Src.dwMask[3] = k16MaskA;

    mask.InitShifters(mask.m_Src);

    EXPECT_EQ(mask.m_Src.dwPos[0], 0u);
    EXPECT_EQ(mask.m_Src.dwPos[1], 5u);
    EXPECT_EQ(mask.m_Src.dwPos[2], 10u);
    EXPECT_EQ(mask.m_Src.dwPos[3], 15u);

    EXPECT_EQ(mask.m_Src.dwSize[0], 3u); // 5-bit channel: 8 - 5 = 3
    EXPECT_EQ(mask.m_Src.dwSize[1], 3u);
    EXPECT_EQ(mask.m_Src.dwSize[2], 3u);
    EXPECT_EQ(mask.m_Src.dwSize[3], 7u); // 1-bit channel: 8 - 1 = 7
}

TEST(ZColorMask, InitShiftersZeroMask)
{
    ZColorMask mask;
    mask.m_Src.dwMask[0] = 0;
    mask.m_Src.dwMask[1] = 0;
    mask.m_Src.dwMask[2] = 0;
    mask.m_Src.dwMask[3] = 0;

    mask.InitShifters(mask.m_Src);

    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(mask.m_Src.dwPos[i], 0u);
        EXPECT_EQ(mask.m_Src.dwSize[i], 8u);
    }
}

TEST(ZColorMask, SrcToDstIdentity)
{
    ZColorMask mask = MakeStdToStdMask();

    EXPECT_EQ(mask.SrcToDst(0xAABBCCDD, false), 0xAABBCCDDu);
    EXPECT_EQ(mask.SrcToDst(0x00000000, false), 0x00000000u);
    EXPECT_EQ(mask.SrcToDst(0xFFFFFFFF, false), 0xFFFFFFFFu);
}

TEST(ZColorMask, SrcToDstDownscalesTo16Bit)
{
    ZColorMask mask = MakeStdToStdMask();
    mask.m_Dst.dwMask[0] = k16MaskR;
    mask.m_Dst.dwMask[1] = k16MaskG;
    mask.m_Dst.dwMask[2] = k16MaskB;
    mask.m_Dst.dwMask[3] = k16MaskA;
    mask.InitShifters(mask.m_Dst);

    EXPECT_EQ(mask.SrcToDst(0xFFFFFFFF, false), 0xFFFFu);
    EXPECT_EQ(mask.SrcToDst(0x00000000, false), 0x0000u);
    // A=0x80, B=0x40, G=0x20, R=0x10 -> A:1<<15, B:8<<10, G:4<<5, R:2
    EXPECT_EQ(mask.SrcToDst(0x80402010, false), 0xA082u);
}

TEST(ZColorMask, SrcToDstUpscalesFrom16Bit)
{
    ZColorMask mask = MakeStdToStdMask();
    mask.m_Src.dwMask[0] = k16MaskR;
    mask.m_Src.dwMask[1] = k16MaskG;
    mask.m_Src.dwMask[2] = k16MaskB;
    mask.m_Src.dwMask[3] = k16MaskA;
    mask.InitShifters(mask.m_Src);

    // 0x1F << 3 = 0xF8 per color channel, 1 << 7 = 0x80 for alpha.
    EXPECT_EQ(mask.SrcToDst(0xFFFF, false), 0x80F8F8F8u);
    EXPECT_EQ(mask.SrcToDst(0x0000, false), 0x00000000u);
}

TEST(ZColorMask, SrcToDstAlphaInvert)
{
    ZColorMask mask = MakeStdToStdMask();

    EXPECT_EQ(mask.SrcToDst(0x80000000, true), 0x7F000000u);
    EXPECT_EQ(mask.SrcToDst(0x00000000, true), 0xFF000000u);
    EXPECT_EQ(mask.SrcToDst(0x12345678, true), 0xED345678u);
    // Non-alpha channels are not affected by the inversion.
    EXPECT_EQ(mask.SrcToDst(0x12345678, false), 0x12345678u);
}

TEST(ZBitmap32, DefaultCtorSetsTypeAndDefaults)
{
    ZBitmap32 bmp;

    EXPECT_EQ(bmp.GetType(), 1380401729); // 'RGBA'
    EXPECT_EQ(bmp.GetSizeX(), 0);
    EXPECT_EQ(bmp.GetSizeY(), 0);
    EXPECT_EQ(bmp.GetMipLevelCount(), 0);
    EXPECT_FLOAT_EQ(bmp.GetScaleFactor(), 1.0f);
}

TEST(ZBitmap32, ParamCtorIdentityMasks)
{
    uint32_t pixels[4] = { 0xDEADBEEF, 0x12345678, 0xAABBCCDD, 0x00000000 };
    ZBitmap32 bmp(pixels, 2, 2, kStdMaskR, kStdMaskG, kStdMaskB, kStdMaskA, false);

    EXPECT_EQ(bmp.GetSizeX(), 2);
    EXPECT_EQ(bmp.GetSizeY(), 2);
    EXPECT_EQ(bmp.GetMipLevelCount(), 1);

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    ASSERT_NE(pMip, nullptr);
    EXPECT_TRUE(pMip->bAllocated);
    EXPECT_EQ(pMip->dwSize, 16u);
    EXPECT_EQ(pMip->dwSizeX, 2);
    EXPECT_EQ(pMip->dwSizeY, 2);
    ASSERT_NE(pMip->pData, nullptr);

    const auto* pData = static_cast<const uint32_t*>(pMip->pData);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(pData[i], pixels[i]);
    }
}

TEST(ZBitmap32, ParamCtorRemapsChannels)
{
    // Source layout is 0xAARRGGBB, internal layout is 0xAABBGGRR.
    uint32_t pixels[1] = { 0x80112233 };
    ZBitmap32 bmp(pixels, 1, 1, 0xFF0000, 0xFF00, 0xFF, 0xFF000000, false);

    const auto* pData = static_cast<const uint32_t*>(bmp.GetMIPLevel(0)->pData);
    EXPECT_EQ(pData[0], 0x80332211u);
}

TEST(ZBitmap32, ParamCtorInvertsAlpha)
{
    uint32_t pixels[2] = { 0x00112233, 0xFF112233 };
    ZBitmap32 bmp(pixels, 2, 1, kStdMaskR, kStdMaskG, kStdMaskB, kStdMaskA, true);

    const auto* pData = static_cast<const uint32_t*>(bmp.GetMIPLevel(0)->pData);
    EXPECT_EQ(pData[0], 0xFF112233u);
    EXPECT_EQ(pData[1], 0x00112233u);
}

TEST(ZBitmap32, GetRGBAReadsBackWithPitch)
{
    uint32_t pixels[4] = { 0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    ZBitmap32 bmp(pixels, 2, 2, kStdMaskR, kStdMaskG, kStdMaskB, kStdMaskA, false);

    uint8_t dst[32] = {};
    bmp.GetRGBA(dst, 0, 0, 2, 2, 16, 0);

    const auto* pRow0 = reinterpret_cast<const uint32_t*>(dst);
    const auto* pRow1 = reinterpret_cast<const uint32_t*>(dst + 16);
    EXPECT_EQ(pRow0[0], 0x11111111u);
    EXPECT_EQ(pRow0[1], 0x22222222u);
    EXPECT_EQ(pRow1[0], 0x33333333u);
    EXPECT_EQ(pRow1[1], 0x44444444u);
}

TEST(ZBitmap32, GetRGBAReadsSubRect)
{
    uint32_t pixels[4] = { 0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    ZBitmap32 bmp(pixels, 2, 2, kStdMaskR, kStdMaskG, kStdMaskB, kStdMaskA, false);

    uint8_t dst[4] = {};
    bmp.GetRGBA(dst, 1, 1, 1, 1, 4, 0);

    EXPECT_EQ(*reinterpret_cast<const uint32_t*>(dst), 0x44444444u);
}

TEST(ZBitmap32, GetDataIdentityMasks)
{
    uint32_t pixels[4] = { 0xDEADBEEF, 0x12345678, 0xAABBCCDD, 0x00000000 };
    ZBitmap32 bmp(pixels, 2, 2, kStdMaskR, kStdMaskG, kStdMaskB, kStdMaskA, false);

    uint32_t out[4] = {};
    ZBitmap::TDataBlock block;
    block.dwSizeX = 2;
    block.dwSizeY = 2;
    block.dwPich = 8;
    block.dwMaskR = kStdMaskR;
    block.dwMaskG = kStdMaskG;
    block.dwMaskB = kStdMaskB;
    block.dwMaskA = kStdMaskA;
    block.dwBitCount = 32;
    block.pData = out;

    bmp.GetData(&block, 0);

    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(out[i], pixels[i]);
    }
}

TEST(ZBitmap32, GetDataConvertsTo16Bit)
{
    uint32_t pixels[2] = { 0xFFFFFFFF, 0x00000000 };
    ZBitmap32 bmp(pixels, 2, 1, kStdMaskR, kStdMaskG, kStdMaskB, kStdMaskA, false);

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
    EXPECT_EQ(out[1], 0x0000);
}

TEST(ZBitmap32, GetDataRespectsOrigin)
{
    uint32_t pixels[4] = { 0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    ZBitmap32 bmp(pixels, 2, 2, kStdMaskR, kStdMaskG, kStdMaskB, kStdMaskA, false);

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

    EXPECT_EQ(out[0], 0x44444444u);
}

TEST(ZBitmap32, PackMipLevelCopiesAndOwnsData)
{
    ZBitmap32 bmp;
    bmp.SetSizeX(2);
    bmp.SetSizeY(2);
    bmp.SetMIPLevels(1);

    uint32_t pixels[4] = { 0x11111111, 0x22222222, 0x33333333, 0x44444444 };
    bmp.PackMipLevel(0, 2, 2, pixels);

    // Source is copied (bAllocate = true), later mutations must not leak into the mip.
    pixels[0] = 0xFFFFFFFF;

    ZBitmap::TMipLevel* pMip = bmp.GetMIPLevel(0);
    EXPECT_TRUE(pMip->bAllocated);
    EXPECT_EQ(pMip->dwSize, 16u);
    EXPECT_EQ(pMip->dwSizeX, 2);
    EXPECT_EQ(pMip->dwSizeY, 2);

    const auto* pData = static_cast<const uint32_t*>(pMip->pData);
    EXPECT_EQ(pData[0], 0x11111111u);
    EXPECT_EQ(pData[1], 0x22222222u);
    EXPECT_EQ(pData[2], 0x33333333u);
    EXPECT_EQ(pData[3], 0x44444444u);
}
