#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Bitmap/ZBitmapPal.h>


namespace Glacier
{
    /**
     * @brief 8-bit paletted bitmap with a separate opacity buffer (type 'PALO').
     *
     * Extends ZBitmapPal with a per-texel 8-bit opacity plane that overrides
     * the palette alpha: GetRGBA() combines the palette RGB with the opacity
     * byte as alpha. The opacity plane is serialized after the palette.
     */
    class ZBitmapPalOpac : public ZBitmapPal
    {
    public:
        // vtbl
        ~ZBitmapPalOpac() override;
        void GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const override;
        uint32_t GetBinSize() override;
        void SaveBin(char* pBuffer) override;
        void LoadBin(const char* pBuffer) override;

        // methods
        ZBitmapPalOpac();

        // members
        uint32_t* m_pOpac { nullptr };
        bool m_bOpacAllocated { false };
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZBitmapPalOpac, 0x44); // PC allocation
}