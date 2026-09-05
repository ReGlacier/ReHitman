#pragma once

namespace Glacier
{
    enum ZTextureType : unsigned int {
        BITMAP_PAL      = 0x50414C4E,
        BITMAP_PAL_OPAC = 0x50414C4F,
        BITMAP_32       = 0x52474241,
        BITMAP_U8V8     = 0x55385638,
        BITMAP_DXT1     = 0x44585431,
        BITMAP_DXT3     = 0x44585433,
        BITMAP_I8       = 0x49382020,

        // Pseudo types assigned from ZBitmap params in ZTextureManagerD3D::CreateTexture
        TEXTURE_EMBM    = 0x454D424D, // 'EMBM' (params & 0x80)
        TEXTURE_CUBE    = 0x43425545, // PC cube marker (params & 0x400); the iOS build uses 'CUBE' = 0x43554245
        TEXTURE_DMAP    = 0x444D4150, // 'DMAP' (params & 0x800)
        TEXTURE_DOT3    = 0x444F5433  // 'DOT3'
    };
}