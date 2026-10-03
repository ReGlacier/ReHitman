#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>


namespace Glacier
{
    /**
     * @brief 16-bit U8V8 bump map bitmap (type 'U8V8').
     *
     * Stores two signed-offset bytes per texel (U in the low byte, V in the
     * high byte), used by the renderer for bump/environment mapping. Packing
     * sources U from the green channel and V from the alpha channel of the
     * input RGBA data; GetRGBA() expands a texel to R=255, G=U, B=V, A=255.
     */
    class ZBitmapU8V8 : public ZBitmap
    {
    public:
        // vtbl
        ~ZBitmapU8V8() override;
        void GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const override;
        void GetData(ZBitmap::TDataBlock* pData, int dwMipLevel) override;
        void Swizzle() override;
        void PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA) override;

        // methods
        ZBitmapU8V8();

        // members
    };
    RE_VERIFY_SIZE(ZBitmapU8V8, 0x30); // Verified PC allocation
}