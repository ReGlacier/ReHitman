#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>


namespace Glacier
{
    class ZBitmapDXT3 : public ZBitmap
    {
    public:
        // vtbl
        ~ZBitmapDXT3() override;
        void GetData(ZBitmap::TDataBlock* pData, int dwMipLevel) override;
        void PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA) override;
        
        // methods
        ZBitmapDXT3();
        void DecodeMipLevel(int* pDst, int dwLevel) const;

        // members
    };
    RE_VERIFY_SIZE(ZBitmapDXT3, 0x30); // Verified PC allocation
}