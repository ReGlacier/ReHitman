#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/ZTextureD3D.h>
#include <Glacier/Render/DLLTEXCON.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief D3D9 texture manager of the Glacier renderer.
     *
     * Owns the global texture pool (MAX_TEXTURES_NR ZTextureD3D slots) and turns
     * packed texture (TEX) buffers produced by the engine into live
     * IDirect3DTexture9 / IDirect3DCubeTexture9 objects. For every occupied slot
     * the created D3D texture object is stored in ZTextureBase::m_pUserData, and
     * animated textures additionally keep a frame table in ZTextureBase::m_pAnimData.
     *
     * @section tex_buffer_format TEX buffer format
     * All offsets are relative to the start of the buffer:
     *  - 12-byte header:
     *      - +0x0 byte offset of the texture offset table (MAX_TEXTURES_NR uint32
     *        entries, one per texture id; 0 means the slot is empty);
     *      - +0x4 byte offset of the per-texture reference table;
     *      - +0x8 id of the reflection texture (stored in m_lReflTextureId).
     *  - Offset table entry: byte offset of a bitmap record.
     *  - Bitmap record: uint32 reserved (not read by the loader), uint32 type
     *    (see ZTextureType), followed by the ZBitmap::LoadBin() payload.
     *  - Reference table entry of an animated texture (bitmap params & 0x100):
     *    uint32 frame count followed by that many texture ids; the whole block
     *    (count + ids) is copied verbatim into ZTextureBase::m_pAnimData.
     *  - Reference table entry of a cube texture (bitmap params & 0x400): 8 bytes
     *    of data followed by the texture ids of the five remaining cube faces
     *    (face 0 is the cube texture record itself).
     *
     * @section tex_formats Supported formats
     * Bitmap types loaded from TEX buffers (ZTextureType):
     *  - BITMAP_PAL ('PALN') and BITMAP_PAL_OPAC ('PALO') - 8-bit paletted
     *    (note: a PAL_OPAC record does not produce a D3D texture on the PC path);
     *  - BITMAP_32 ('RGBA') - uncompressed 32-bit RGBA;
     *  - BITMAP_U8V8 ('U8V8') - 16-bit bump offsets, uploaded as D3DFMT_A8L8;
     *  - BITMAP_I8 ('I8  ') - 8-bit intensity, uploaded as D3DFMT_L8;
     *  - BITMAP_DXT1 / BITMAP_DXT3 - uploaded as D3DFMT_DXT1 / D3DFMT_DXT3 when
     *    the device supports them and DXT is not disabled, otherwise decoded
     *    through the 16/32-bit RGBA fallback of CreateRGBATexture().
     *
     * Pseudo types derived from the bitmap params in CreateTexture():
     *  - params & 0x80  -> EMBM bump map (CreateEMBMTexture(), falls back to RGBA
     *    when the device has no EMBM format or EMBM is disabled);
     *  - params & 0x400 -> cube texture (CreateCubeTexture() from six bitmaps);
     *  - params & 0x800 -> displacement map (CreateDMAPTexture(), D3DFMT_L16
     *    with D3DUSAGE_DMAP);
     *  - 'DOT3' records -> normal map (CreateDOT3Texture(), D3DFMT_A8R8G8B8).
     *
     * The iOS build additionally handles DXT5, A8, 16-bit RGBA and ASTC bitmaps;
     * those paths do not exist in the PC build and are intentionally not mirrored.
     */
    class ZTextureManagerD3D : public DLLTEXCON
    {
    public:
        // constants
        static constexpr size_t MAX_TEXTURES_NR = 2048;

        // vtbl
        ~ZTextureManagerD3D() override;

        /**
         * @brief Returns the texture for the given slot, resolving animation frames.
         * @param lTextureId texture slot id; 0 or MAX_TEXTURES_NR selects the global white texture
         * @param lFrameId animation frame index (asserted to be in range for animated textures)
         * @return ZTextureD3D* texture from the pool or &g_texWhite
         */
        ZTextureD3D* GetTexture(uint32_t lTextureId, uint32_t lFrameId) override;

        /**
         * @brief Releases the current pool and installs a packed TEX buffer.
         *
         * Frees all previously installed textures, then walks the offset table,
         * LoadBin()s every bitmap record and creates the matching D3D texture.
         * Cube textures (params & 0x400) gather their six faces and animated
         * textures (params & 0x100) get their frame table copied into m_pAnimData.
         * @param pTextureBuffer packed TEX buffer (see the class docs for the layout)
         */
        virtual void InstallTextureBuffer(void* pTextureBuffer);

        /**
         * @brief Releases every D3D texture and animation table in the pool.
         */
        virtual void FreeTextureBuffer();

        /**
         * @brief Fills a pool slot from a bitmap (array form, used for cube faces).
         *
         * Writes id/name/size/scale/offset fields and dispatches to one of the
         * Create*Texture() helpers based on the bitmap type and params; the
         * created D3D texture object is stored in pTexture->m_pUserData.
         * @param pBitmaps source bitmaps (pBitmaps[0] describes the texture)
         * @param pTexture destination pool slot
         */
        virtual void CreateTexture(const ZBitmap** pBitmaps, ZTextureD3D* pTexture);

        /**
         * @brief Fills a pool slot from a single bitmap; forwards to the array overload.
         * @param pBitmap source bitmap
         * @param pTexture destination pool slot
         */
        virtual void CreateTexture(const ZBitmap *pBitmap, ZTextureD3D* pTexture);

        /**
         * @brief Reserves the first free pool slot (marks it with a sentinel).
         * @return uint32_t reserved texture id, or 0 when the pool is full
         */
        virtual uint32_t ReserveTexture();

        /**
         * @brief Re-uploads mip level 0 of an existing D3D texture from a bitmap.
         * @param pBitmap source bitmap
         * @param pTexture pool slot whose m_pUserData is updated
         */
        virtual void UpdateTexture(const ZBitmap* pBitmap, ZTextureD3D* pTexture);

        /**
         * @brief Probes the device and caches the supported texture formats.
         * @param TargetFormat adapter format used for the CheckDeviceFormat() calls
         */
        virtual void CheckFormats(D3DFORMAT TargetFormat);

        /**
         * @brief Checks whether any EMBM (bump map) format is available.
         * @return bool true when m_ddpfEMBM[0] holds a usable format
         */
        virtual bool HasEMBM() const;

        /**
         * @brief Checks whether the EMBM format carries a luminance channel.
         * @return bool true for D3DFMT_L6V5U5 or D3DFMT_X8L8V8U8
         */
        virtual bool HasEMBMUVL() const;

        /**
         * @brief Checks whether DXT1 compression is available.
         * @return bool true when m_ddpfCompressed[0] holds D3DFMT_DXT1
         */
        virtual bool HasDXT() const;

        /**
         * @brief Checks whether the given DXTn (1-based) format is available.
         * @param lDxtNr DXT format number (1..5), asserted non-zero
         * @return bool true when the format is supported
         */
        virtual bool HasDXT_Nr(uint8_t lDxtNr) const;

        // methods
        ZTextureManagerD3D(ZDirect3DDevice* pDevice);

        /**
         * @brief Creates a cube texture from six face bitmaps (order: -Z, +Z, -X, +X, +Y, -Y).
         * @param ppBitmaps six face bitmaps; all must be square and equally sized
         * @return IDirect3DCubeTexture9* created texture, or nullptr on size mismatch
         */
        IDirect3DCubeTexture9* CreateCubeTexture(const ZBitmap** ppBitmaps);

        /**
         * @brief Creates a D3DFMT_L8 texture from an 8-bit intensity bitmap.
         * @param pBitmap source bitmap
         * @return IDirect3DTexture9* created texture
         */
        IDirect3DTexture9* CreateI8Texture(const ZBitmap* pBitmap);

        /**
         * @brief Creates a D3DFMT_A8L8 texture from a U8V8 bump offset bitmap.
         * @param pBitmap source bitmap
         * @return IDirect3DTexture9* created texture, or nullptr on failure
         */
        IDirect3DTexture9* CreateU8V8Texture(const ZBitmap* pBitmap);

        /**
         * @brief Creates an uncompressed RGBA texture (32-bit, or 16-bit for DXT fallbacks).
         *
         * Textures larger than the device limit are rebuilt through a mipmapped
         * ZBitmap32 copy and clamped to the first mip level that fits.
         * @param pBitmap source bitmap
         * @return IDirect3DTexture9* created texture
         */
        IDirect3DTexture9* CreateRGBATexture(const ZBitmap* pBitmap);

        /**
         * @brief Creates a DXT1/DXT3 compressed texture from a DXT bitmap.
         *
         * Requires power-of-two dimensions (clamped up to 4x4); raw compressed
         * mip data is copied as-is.
         * @param pBitmap source bitmap
         * @return IDirect3DTexture9* created texture, or nullptr for non-POT input
         */
        IDirect3DTexture9* CreateDXTTexture(const ZBitmap* pBitmap);

        /**
         * @brief Creates a D3DFMT_A8R8G8B8 normal map texture from a height field bitmap.
         * @param pBitmap source bitmap
         * @return IDirect3DTexture9* created texture
         */
        IDirect3DTexture9* CreateDOT3Texture(const ZBitmap* pBitmap);

        /**
         * @brief Creates a D3DFMT_L16 displacement map texture (D3DUSAGE_DMAP).
         * @param pBitmap source bitmap
         * @return IDirect3DTexture9* created texture
         */
        IDirect3DTexture9* CreateDMAPTexture(const ZBitmap* pBitmap);

        /**
         * @brief Creates an EMBM bump map texture in the probed m_ddpfEMBM[0] format.
         * @param pBitmap source bitmap
         * @return IDirect3DTexture9* created texture
         */
        IDirect3DTexture9* CreateEMBMTexture(const ZBitmap* pBitmap);

        // members
        ZTextureD3D m_Textures[MAX_TEXTURES_NR];  ///< global texture pool
        ZDirect3DDevice* m_pDev { nullptr };      ///< owning D3D device wrapper
        D3DFORMAT m_ddpfCompressed[5];            ///< supported DXT1..DXT5 formats (D3DFMT_UNKNOWN when unsupported)
        D3DFORMAT m_ddpfPalette[1];               ///< supported paletted format (D3DFMT_P8)
        D3DFORMAT m_ddpfRGBA32[2];                ///< supported 32-bit RGBA formats
        D3DFORMAT m_ddpfRGBA16[2];                ///< supported 16-bit RGBA formats (DXT1/DXT3 fallbacks)
        D3DFORMAT m_ddpfEMBM[1];                  ///< supported EMBM bump format
        uint32_t m_lMaxWidth{4096};               ///< device MaxTextureWidth
        uint32_t m_lMaxHeight{4096};              ///< device MaxTextureHeight
        uint32_t m_lReflTextureId{0};             ///< id of the reflection texture from the installed TEX buffer
    };
    RE_VERIFY_SIZE(ZTextureManagerD3D, 0x22040); // Verified PC allocation
    RE_VERIFY_OFFSET(ZTextureManagerD3D, m_pDev, 0x22004); // PC
}
