#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>


namespace Glacier
{
    struct ZColorMask
    {
        // types
        struct _ZColorMask
        {
            uint32_t dwMask[4];
            uint32_t dwPos[4];
            uint32_t dwSize[4];
        };
        RE_VERIFY_SIZE(_ZColorMask, 0x30);
        
        // methods
        ZColorMask();
        uint32_t SrcToDst(uint32_t dwColor, bool bAlphaInvert);
        void InitShifters(ZColorMask::_ZColorMask& Mask);

        // members
        _ZColorMask m_Src;
        _ZColorMask m_Dst;
        float m_Res[4];
    };
    RE_VERIFY_SIZE(ZColorMask, 0x70);

    /**
     * @brief Uncompressed 32-bit RGBA bitmap (type 'RGBA').
     *
     * Stores raw 32-bit pixels, 4 bytes per texel in R|G<<8|B<<16|A<<24 order.
     * This is the reference uncompressed format: PackMipLevel() keeps the
     * source RGBA data as-is and GetRGBA() is a plain copy.
     */
    class ZBitmap32 : public ZBitmap
    {
    public:
        // vtbl
        ~ZBitmap32() override;
        void GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const override;
        void GetData(ZBitmap::TDataBlock* pData, int dwMipLevel) override;
        void PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA) override;

        // methods
        ZBitmap32();
        ZBitmap32(uint32_t* pData, int x, int y, int rMask, int gMask, int bMask, int aMask, bool bInvertAlpha);
    };
    RE_VERIFY_SIZE(ZBitmap32, 0x30); // Verified PC allocation
}