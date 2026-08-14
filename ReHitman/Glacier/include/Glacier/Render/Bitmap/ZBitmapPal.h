#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>


namespace Glacier
{
    /**
     * @brief 8-bit paletted bitmap (type 'PALN').
     *
     * Stores one palette index byte per texel plus a palette of 32-bit RGBA
     * colors that is serialized together with the mip data (see SaveBin() /
     * LoadBin()). Texel expansion is a plain palette lookup. Create() is not
     * supported for this format.
     */
    class ZBitmapPal : public ZBitmap
    {
    public:
        // vtbl
        ~ZBitmapPal() override;
        void Create(const ZBitmap& Bitmap, bool bBitmap) override;
        void GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const override;
        void GetData(ZBitmap::TDataBlock* pData, int dwMipLevel) override;
        uint32_t GetBinSize() override;
        void SaveBin(char* pBuffer) override;
        void LoadBin(const char* pBuffer) override;
        void Swizzle() override;
        void PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA) override;
        virtual int32_t GetPalSize() const;
        virtual uint32_t* GetPal() const;
        virtual void SetPalette(uint32_t* pPalette, int32_t lPaletteSize, bool bNeedCopyPalette);

        // methods
        ZBitmapPal();

        // members
        uint32_t* m_pPal { nullptr };
        bool m_bPalAllocated { false };
        RE_ADD_PADDING(3);
        int32_t m_lPalSize { 0 };
    };
    RE_VERIFY_SIZE(ZBitmapPal, 0x3C); // Verified PC allocation
    RE_VERIFY_OFFSET(ZBitmapPal, m_pPal, 0x30); // Verified
    RE_VERIFY_OFFSET(ZBitmapPal, m_lPalSize, 0x38); // Verified
}