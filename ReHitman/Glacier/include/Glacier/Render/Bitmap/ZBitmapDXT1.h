#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>


namespace Glacier
{
    class ZBitmapDXT1 : public ZBitmap
    {
    public:
        // vtbl
        ~ZBitmapDXT1() override;
        void GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const override;
        void GetData(ZBitmap::TDataBlock* pData, int dwMipLevel) override;
        void PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA) override;
        
        // methods
        ZBitmapDXT1();
        void DecodeMipLevel(int* pDst, int dwLevel) const;

        // members
    };
    RE_VERIFY_SIZE(ZBitmapDXT1, 0x30); // Verified PC allocation
}