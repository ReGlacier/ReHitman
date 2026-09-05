#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>


namespace Glacier
{
    /**
     * @brief 8-bit intensity bitmap (type 'I8  ').
     *
     * Stores one intensity byte per texel. Intensity is sourced from the blue
     * channel on packing and expanded back to grayscale on read: GetRGBA()
     * replicates the value into all four channels, GetData() produces an
     * opaque grayscale pixel (R=G=B=value, A=255).
     */
    class ZBitmapI8 : public ZBitmap
    {
    public:
        // vtbl
        ~ZBitmapI8() override;
        void GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const override;
        void GetData(ZBitmap::TDataBlock* pData, int dwMipLevel) override;
        void Swizzle() override;
        void PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA) override;

        // methods
        ZBitmapI8();
    };
    RE_VERIFY_SIZE(ZBitmapI8, 0x30); // Verified PC allocation
}