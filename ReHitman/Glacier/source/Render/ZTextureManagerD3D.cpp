#include <Glacier/Render/ZTextureManagerD3D.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/Render/Bitmap/ZBitmapU8V8.h>
#include <Glacier/Render/Bitmap/ZBitmapI8.h>
#include <Glacier/Render/Bitmap/ZBitmapPal.h>
#include <Glacier/Render/Bitmap/ZBitmapPalOpac.h>
#include <Glacier/Render/Bitmap/ZBitmapDXT1.h>
#include <Glacier/Render/Bitmap/ZBitmapDXT3.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZTextureType.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        /**
         * @brief Header of a packed texture (TEX) buffer consumed by ZTextureManagerD3D::InstallTextureBuffer().
         *
         * All offsets are relative to the start of the buffer.
         */
        struct STextureBufferHeader
        {
            uint32_t m_lOffsetTableOffset;  ///< Byte offset to the texture offset table (MAX_TEXTURES_NR uint32 entries, 0 = slot is empty)
            uint32_t m_lAnimRefTableOffset; ///< Byte offset to the per-texture reference table (animation frame lists / cube face id lists)
            uint32_t m_lReflTextureId;      ///< Id of the reflection texture
        };

        /**
         * @brief Bitmap record header stored in a TEX buffer.
         *
         * Each non-empty offset table entry points at one of these; the
         * ZBitmap::LoadBin() payload follows right after it.
         */
        struct STextureBufferEntry
        {
            uint32_t m_lReserved;   ///< Not read by the loader
            uint32_t m_dwType;      ///< ZTextureType of the stored bitmap
            // char m_pBinData[];   ///< ZBitmap::LoadBin() payload
        };

        // D3DCUBEMAP_FACES order of the six bitmaps passed to ZTextureManagerD3D::CreateCubeTexture (PC .data:0x763DB4)
        constexpr D3DCUBEMAP_FACES s_CubeFaces[6] =
        {
            D3DCUBEMAP_FACE_NEGATIVE_Z,
            D3DCUBEMAP_FACE_POSITIVE_Z,
            D3DCUBEMAP_FACE_NEGATIVE_X,
            D3DCUBEMAP_FACE_POSITIVE_X,
            D3DCUBEMAP_FACE_POSITIVE_Y,
            D3DCUBEMAP_FACE_NEGATIVE_Y
        };

        // (R + G + B) / 3 of a 32-bit RGBA pixel (PC: integer division)
        uint32_t AverageRGB(uint32_t dwPixel)
        {
            return ((dwPixel & 0xFF) + ((dwPixel >> 8) & 0xFF) + ((dwPixel >> 16) & 0xFF)) / 3;
        }

        // Packs a float normal component into a byte: clamp(v * 127 + 128, 0, 255) (PC sub_48C380)
        uint32_t PackNormalComponent(float v)
        {
            v = v * 127.0f + 128.0f;
            if (v < 0.0f)
            {
                return 0;
            }
            if (v > 255.0f)
            {
                v = 255.0f;
            }
            return static_cast<uint32_t>(v);
        }

        /**
         * @brief Converts a 32-bit RGBA height field into a DOT3 normal map (PC sub_48C5A0, iOS MakeNormalMap).
         *
         * For every texel the normal is rebuilt from the intensity deltas of the
         * pixels above and to the right; the alpha becomes 127 + srcAlpha / 2.
         */
        void MakeNormalMap(uint32_t* pDst, const uint32_t* pSrc, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwSrcPitch, uint32_t dwDstPitch)
        {
            const uint32_t* pSrcRow = pSrc;
            uint32_t* pDstRow = pDst;

            for (uint32_t y = 0; y < dwHeight; ++y)
            {
                const uint32_t* pPrevRow = y ? reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(pSrcRow) - dwSrcPitch) : pSrcRow;

                for (uint32_t x = 0; x < dwWidth; ++x)
                {
                    const uint32_t xNext = x ? x + 1 : 0;

                    const uint32_t dwBase = pPrevRow[xNext];
                    const int lIntensity = AverageRGB(dwBase);
                    const float fDX = static_cast<float>(static_cast<int>(AverageRGB(pPrevRow[x])) - lIntensity) * (1.0f / 255.0f);
                    const float fDY = static_cast<float>(static_cast<int>(AverageRGB(pSrcRow[xNext])) - lIntensity) * (1.0f / 255.0f);

                    const float fInvLen = 1.0f / sqrtf(fDY * fDY + fDX * fDX + 1.0f);

                    const uint32_t dwNormal = (PackNormalComponent(-fDY * fInvLen) << 16)
                                            | (PackNormalComponent(-fDX * fInvLen) << 8)
                                            | PackNormalComponent(fInvLen);

                    pDstRow[x] = dwNormal | (((dwBase >> 1) & 0x7F000000) + 0x7F000000);
                }

                pSrcRow = reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(pSrcRow) + dwSrcPitch);
                pDstRow = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(pDstRow) + dwDstPitch);
            }
        }

        /**
         * @brief Converts a 32-bit RGBA height field into an L16 displacement map (PC sub_48C650).
         *
         * Every texel becomes the average of the R/G/B channels stored in the
         * high byte of a 16-bit luminance value.
         */
        void MakeDMap(uint16_t* pDst, const uint32_t* pSrc, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwSrcPitch, uint32_t dwDstPitch)
        {
            const uint8_t* pSrcRow = reinterpret_cast<const uint8_t*>(pSrc);
            uint8_t* pDstRow = reinterpret_cast<uint8_t*>(pDst);

            for (uint32_t y = 0; y < dwHeight; ++y)
            {
                for (uint32_t x = 0; x < dwWidth; ++x)
                {
                    reinterpret_cast<uint16_t*>(pDstRow)[x] = static_cast<uint16_t>(AverageRGB(reinterpret_cast<const uint32_t*>(pSrcRow)[x]) << 8);
                }

                pSrcRow += dwSrcPitch;
                pDstRow += dwDstPitch;
            }
        }

        /**
         * @brief Converts a 32-bit RGBA height field into an EMBM bump map (PC sub_48C6D0, iOS MakeBumpMap).
         *
         * Produces signed U/V slopes per texel in the D3DFMT_V8U8, D3DFMT_L6V5U5
         * or D3DFMT_X8L8V8U8 layout; the L channel is taken from the source alpha.
         */
        void MakeBumpMap(void* pDst, const uint32_t* pSrc, uint32_t dwWidth, uint32_t dwHeight, D3DFORMAT format)
        {
            const uint32_t dwSrcPitch = 4 * dwWidth;
            const uint32_t dwDstPitch = (format == D3DFMT_X8L8V8U8 ? 4 : 2) * dwWidth;

            const uint32_t* pSrcRow = pSrc;
            uint8_t* pDstRow = static_cast<uint8_t*>(pDst);

            for (uint32_t y = 0; y < dwHeight; ++y)
            {
                const uint32_t* pPrevRow = y ? reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(pSrcRow) - dwSrcPitch) : pSrcRow;
                uint8_t* pOut = pDstRow;

                for (uint32_t x = 0; x < dwWidth; ++x)
                {
                    const uint32_t xPrev = x ? x - 1 : 0;

                    const int lIntensity = AverageRGB(pSrcRow[x]);
                    const int lDU = static_cast<int>(AverageRGB(pPrevRow[x])) - lIntensity;
                    const int lDV = static_cast<int>(AverageRGB(pSrcRow[xPrev])) - lIntensity;

                    if (format == D3DFMT_V8U8)
                    {
                        pOut[0] = static_cast<uint8_t>(lDU);
                        pOut[1] = static_cast<uint8_t>(lDV);
                        pOut += 2;
                    }
                    else
                    {
                        const uint32_t dwLumSrc = pPrevRow[xPrev];
                        if (format == D3DFMT_L6V5U5)
                        {
                            *reinterpret_cast<uint16_t*>(pOut) = static_cast<uint16_t>(
                                ((dwLumSrc >> 16) & 0xFC00) | ((static_cast<uint8_t>(lDU)) >> 3) | (32 * (static_cast<uint8_t>(lDV) >> 3)));
                            pOut += 2;
                        }
                        else if (format == D3DFMT_X8L8V8U8)
                        {
                            pOut[0] = static_cast<uint8_t>(lDU);
                            pOut[1] = static_cast<uint8_t>(lDV);
                            pOut[2] = static_cast<uint8_t>(dwLumSrc >> 24);
                            pOut[3] = 0;
                            pOut += 4;
                        }
                    }
                }

                pSrcRow = reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(pSrcRow) + dwSrcPitch);
                pDstRow += dwDstPitch;
            }
        }
    }

    ZTextureManagerD3D::ZTextureManagerD3D(ZDirect3DDevice* pDevice)
    {
        m_pDev = pDevice;
        memset(&m_Textures, 0, sizeof(m_Textures));

        D3DCAPS9 devCaps {};
        g_pd3dInterface->GetDeviceCaps(0, D3DDEVTYPE_HAL, &devCaps);

        m_lMaxWidth = devCaps.MaxTextureWidth;
        m_lMaxHeight = devCaps.MaxTextureHeight;
    }

    ZTextureManagerD3D::~ZTextureManagerD3D()
    {
        FreeTextureBuffer();
    }

    ZTextureD3D* ZTextureManagerD3D::GetTexture(uint32_t lTextureId, uint32_t lFrameId)
    {
        ZTextureD3D* pTexWhite = &g_texWhite;

        if (!lTextureId || lTextureId == MAX_TEXTURES_NR)
        {
            return pTexWhite;
        }

        ZTextureD3D* pTexture = lTextureId < MAX_TEXTURES_NR ? &m_Textures[lTextureId] : &m_Textures[lTextureId - MAX_TEXTURES_NR];
        if (!pTexture)
        {
            return pTexWhite;
        }

        if (pTexture->m_pAnimData)
        {
            ZASSERT(lFrameId < pTexture->m_pAnimData[0]);
            pTexture = &m_Textures[pTexture->m_pAnimData[lFrameId + 1]];
        }

        if (!pTexture)
        {
            return pTexWhite;
        }

        return pTexture;
    }
    
    void ZTextureManagerD3D::InstallTextureBuffer(void* pTextureBuffer)
    {
        FreeTextureBuffer(); // unload previously loaded TEX files from memory

        char* pBuffer = static_cast<char*>(pTextureBuffer);
        const auto* pHeader = static_cast<const STextureBufferHeader*>(pTextureBuffer);

        char* pOffsetTable = pBuffer + pHeader->m_lOffsetTableOffset;
        char* pAnimRefTable = pBuffer + pHeader->m_lAnimRefTableOffset;
        m_lReflTextureId = pHeader->m_lReflTextureId;

        ZBitmap32 bitmap32;
        ZBitmapU8V8 bitmapU8V8;
        ZBitmapI8 bitmapI8;
        ZBitmapPal bitmapPal;
        ZBitmapPalOpac bitmapPalOpac;
        ZBitmapDXT1 bitmapDXT1;
        ZBitmapDXT3 bitmapDXT3;

        for (uint32_t lIndex = 0; lIndex < MAX_TEXTURES_NR; ++lIndex)
        {
            const uint32_t lOffset = reinterpret_cast<uint32_t*>(pOffsetTable)[lIndex];
            if (!lOffset)
            {
                continue;
            }

            const auto* pEntry = reinterpret_cast<const STextureBufferEntry*>(pBuffer + lOffset);
            const char* pBitmapData = reinterpret_cast<const char*>(pEntry + 1);
            const uint32_t dwType = pEntry->m_dwType;

            ZBitmap* pBitmap = nullptr;
            if (dwType > BITMAP_PAL)
            {
                switch (dwType)
                {
                case BITMAP_PAL_OPAC:
                    bitmapPalOpac.LoadBin(pBitmapData);
                    pBitmap = &bitmapPalOpac;
                    break;
                case BITMAP_32:
                    bitmap32.LoadBin(pBitmapData);
                    pBitmap = &bitmap32;
                    break;
                case BITMAP_U8V8:
                    bitmapU8V8.LoadBin(pBitmapData);
                    pBitmap = &bitmapU8V8;
                    break;
                default:
                    ZASSERT(false);
                    break;
                }
            }
            else if (dwType == BITMAP_PAL)
            {
                bitmapPal.LoadBin(pBitmapData);
                pBitmap = &bitmapPal;
            }
            else if (dwType == BITMAP_DXT1)
            {
                bitmapDXT1.LoadBin(pBitmapData);
                pBitmap = &bitmapDXT1;
            }
            else if (dwType == BITMAP_DXT3)
            {
                bitmapDXT3.LoadBin(pBitmapData);
                pBitmap = &bitmapDXT3;
            }
            else if (dwType == BITMAP_I8)
            {
                bitmapI8.LoadBin(pBitmapData);
                pBitmap = &bitmapI8;
            }
            else
            {
                ZASSERT(false);
            }

            if (pBitmap->GetParams() & 0x400)
            {
                // Cube texture: face 0 is the texture itself, the other 5 faces are referenced by id
                ZBitmap* pBitmaps[6] {};
                pBitmaps[0] = pBitmap;

                const auto* pFaceIds = reinterpret_cast<const uint32_t*>(
                    pBuffer + *reinterpret_cast<const uint32_t*>(pAnimRefTable + 4 * pBitmap->GetId()) + 8);

                for (int lFace = 1; lFace < 6; ++lFace)
                {
                    uint32_t lFaceOffset = reinterpret_cast<uint32_t*>(pOffsetTable)[pFaceIds[lFace - 1]];
                    if (!lFaceOffset)
                    {
                        lFaceOffset = lOffset;
                    }

                    const auto* pFaceEntry = reinterpret_cast<const STextureBufferEntry*>(pBuffer + lFaceOffset);
                    const char* pFaceData = reinterpret_cast<const char*>(pFaceEntry + 1);
                    const uint32_t dwFaceType = pFaceEntry->m_dwType;

                    ZBitmap* pFace = nullptr;
                    if (dwFaceType > BITMAP_PAL)
                    {
                        switch (dwFaceType)
                        {
                        case BITMAP_PAL_OPAC:
                            pFace = ZUniMemory::New<ZBitmapPalOpac>();
                            break;
                        case BITMAP_32:
                            pFace = ZUniMemory::New<ZBitmap32>();
                            break;
                        case BITMAP_U8V8:
                            pFace = ZUniMemory::New<ZBitmapU8V8>();
                            break;
                        default:
                            ZASSERT(false);
                            break;
                        }
                    }
                    else if (dwFaceType == BITMAP_PAL)
                    {
                        pFace = ZUniMemory::New<ZBitmapPal>();
                    }
                    else if (dwFaceType == BITMAP_DXT1)
                    {
                        pFace = ZUniMemory::New<ZBitmapDXT1>();
                    }
                    else if (dwFaceType == BITMAP_DXT3)
                    {
                        pFace = ZUniMemory::New<ZBitmapDXT3>();
                    }
                    else if (dwFaceType == BITMAP_I8)
                    {
                        pFace = ZUniMemory::New<ZBitmapI8>();
                    }
                    else
                    {
                        ZASSERT(false);
                    }

                    pBitmaps[lFace] = pFace;
                    pFace->LoadBin(pFaceData);
                }

                CreateTexture(const_cast<const ZBitmap**>(pBitmaps), &m_Textures[pBitmaps[0]->GetId()]);

                for (int lFace = 1; lFace < 6; ++lFace)
                {
                    if (pBitmaps[lFace])
                    {
                        ZUniMemory::Delete(pBitmaps[lFace]);
                    }
                }
            }
            else
            {
                CreateTexture(pBitmap, &m_Textures[pBitmap->GetId()]);

                if (pBitmap->GetParams() & 0x100)
                {
                    const auto* pAnimData = reinterpret_cast<const uint32_t*>(
                        pBuffer + *reinterpret_cast<const uint32_t*>(pAnimRefTable + 4 * pBitmap->GetId()));

                    auto* pAnimCopy = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * pAnimData[0] + 4));
                    if (pAnimData[0] != 0xFFFFFFFF)
                    {
                        for (uint32_t lFrame = 0; lFrame < pAnimData[0] + 1; ++lFrame)
                        {
                            pAnimCopy[lFrame] = pAnimData[lFrame];
                        }
                    }

                    m_Textures[pBitmap->GetId()].m_pAnimData = pAnimCopy;
                }
            }
        }
    }

    void ZTextureManagerD3D::FreeTextureBuffer()
    {
        for (uint32_t lIndex = 0; lIndex < MAX_TEXTURES_NR; ++lIndex)
        {
            if (m_Textures[lIndex].m_pUserData)
            {
                static_cast<IDirect3DTexture9*>(m_Textures[lIndex].m_pUserData)->Release();
            }

            if (m_Textures[lIndex].m_pAnimData)
            {
                ZUniMemory::Free(m_Textures[lIndex].m_pAnimData);
            }
        }

        memset(m_Textures, 0, sizeof(m_Textures));
    }
    
    void ZTextureManagerD3D::CreateTexture(const ZBitmap** pBitmaps, ZTextureD3D* pTexture)
    {
        ZBitmap* pBitmap = const_cast<ZBitmap*>(*pBitmaps);

        pTexture->m_lTextureId = pBitmap->GetId();
        pTexture->m_fScaleFactor = pBitmap->GetScaleFactor();
        pTexture->SetName(pBitmap->GetName());
        pTexture->m_Format = ZPF8888;
        pTexture->m_dwDataSize = 0;
        pTexture->m_fOffset[0] = 0.0f / pBitmap->GetSizeX();
        pTexture->m_fOffset[1] = 0.0f / pBitmap->GetSizeY();
        pTexture->m_usSize[0] = pBitmap->GetSizeX();
        pTexture->m_usSize[1] = pBitmap->GetSizeY();

        ZTextureType textureType = static_cast<ZTextureType>(pBitmap->GetType());
        if (pBitmap->GetParams() & 0x80)
        {
            textureType = TEXTURE_EMBM;
        }
        if (pBitmap->GetParams() & 0x400)
        {
            textureType = TEXTURE_CUBE;
        }
        if (pBitmap->GetParams() & 0x800)
        {
            textureType = TEXTURE_DMAP;
        }

        if (textureType > TEXTURE_EMBM)
        {
            if (textureType > BITMAP_32)
            {
                if (textureType == BITMAP_U8V8)
                {
                    pTexture->m_pUserData = CreateU8V8Texture(pBitmap);
                }
                return;
            }
            if (textureType != BITMAP_32)
            {
                if (textureType == BITMAP_I8)
                {
                    pTexture->m_pUserData = CreateI8Texture(pBitmap);
                    return;
                }
                if (textureType != BITMAP_PAL)
                {
                    return;
                }
            }
            pTexture->m_pUserData = CreateRGBATexture(pBitmap);
            return;
        }
        if (textureType == TEXTURE_EMBM)
        {
            if (m_ddpfEMBM[0] != D3DFMT_UNKNOWN && !g_pRenderDll->m_bDisableEMBM)
            {
                pTexture->m_pUserData = CreateEMBMTexture(pBitmap);
                return;
            }
            pTexture->m_pUserData = CreateRGBATexture(pBitmap);
            return;
        }
        if (textureType > TEXTURE_DOT3)
        {
            if (textureType == BITMAP_DXT1)
            {
                if (m_ddpfCompressed[0] == D3DFMT_DXT1 && !g_pRenderDll->m_bDisableDXT)
                {
                    pTexture->m_pUserData = CreateDXTTexture(pBitmap);
                    return;
                }
            }
            else if (textureType == BITMAP_DXT3)
            {
                if (m_ddpfCompressed[2] == D3DFMT_DXT3 && !g_pRenderDll->m_bDisableDXT)
                {
                    pTexture->m_pUserData = CreateDXTTexture(pBitmap);
                    return;
                }
            }
            else
            {
                return;
            }
            pTexture->m_pUserData = CreateRGBATexture(pBitmap);
            return;
        }
        switch (textureType)
        {
        case TEXTURE_DOT3:
            pTexture->m_pUserData = CreateDOT3Texture(pBitmap);
            break;
        case TEXTURE_CUBE:
            pTexture->m_pUserData = CreateCubeTexture(pBitmaps);
            break;
        case TEXTURE_DMAP:
            pTexture->m_pUserData = CreateDMAPTexture(pBitmap);
            break;
        default:
            break;
        }
    }

    void ZTextureManagerD3D::CreateTexture(const ZBitmap *pBitmap, ZTextureD3D* pTexture)
    {
        CreateTexture(&pBitmap, pTexture);
    }

    uint32_t ZTextureManagerD3D::ReserveTexture()
    {
        uint32_t lIndex = 1;
        while (m_Textures[lIndex].m_pUserData)
        {
            if (++lIndex >= MAX_TEXTURES_NR)
            {
                return 0;
            }
        }

        m_Textures[lIndex].m_pUserData = reinterpret_cast<void*>(1);
        return lIndex;
    }
    
    void ZTextureManagerD3D::UpdateTexture(const ZBitmap* pBitmap, ZTextureD3D* pTexture)
    {
        ZBitmap::TMipLevel* pMipLevel = const_cast<ZBitmap*>(pBitmap)->GetMIPLevel(0);
        IDirect3DTexture9* pD3DTexture = static_cast<IDirect3DTexture9*>(pTexture->m_pUserData);

        D3DLOCKED_RECT lockedRect;
        pD3DTexture->LockRect(0, &lockedRect, nullptr, 0);

        ZBitmap::TDataBlock dataBlock;
        dataBlock.dwPosX = 0;
        dataBlock.dwPosY = 0;
        dataBlock.dwSizeX = pMipLevel->dwSizeX;
        dataBlock.dwSizeY = pMipLevel->dwSizeY;
        dataBlock.dwPich = lockedRect.Pitch;
        dataBlock.dwMaskR = 0xFF0000;
        dataBlock.dwMaskG = 0xFF00;
        dataBlock.dwMaskB = 0xFF;
        dataBlock.dwMaskA = 0xFF000000;
        dataBlock.dwBitCount = 32;
        dataBlock.bInvertAlpha = false;
        dataBlock.pData = lockedRect.pBits;

        const_cast<ZBitmap*>(pBitmap)->GetData(&dataBlock, 0);

        pD3DTexture->UnlockRect(0);
    }

    void ZTextureManagerD3D::CheckFormats(D3DFORMAT TargetFormat)
    {
        m_ddpfCompressed[0] = D3DFMT_UNKNOWN;
        m_ddpfCompressed[1] = D3DFMT_UNKNOWN;
        m_ddpfCompressed[2] = D3DFMT_UNKNOWN;
        m_ddpfCompressed[3] = D3DFMT_UNKNOWN;
        m_ddpfCompressed[4] = D3DFMT_UNKNOWN;
        m_ddpfPalette[0] = D3DFMT_UNKNOWN;
        m_ddpfRGBA32[0] = D3DFMT_UNKNOWN;
        m_ddpfRGBA32[1] = D3DFMT_UNKNOWN;
        m_ddpfRGBA16[0] = D3DFMT_UNKNOWN;
        m_ddpfRGBA16[1] = D3DFMT_UNKNOWN;
        m_ddpfEMBM[0] = D3DFMT_UNKNOWN;

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1)))
        {
            m_ddpfCompressed[0] = D3DFMT_DXT1;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT2)))
        {
            m_ddpfCompressed[1] = D3DFMT_DXT2;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3)))
        {
            m_ddpfCompressed[2] = D3DFMT_DXT3;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT4)))
        {
            m_ddpfCompressed[3] = D3DFMT_DXT4;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT5)))
        {
            m_ddpfCompressed[4] = D3DFMT_DXT5;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A1R5G5B5)))
        {
            m_ddpfRGBA16[0] = D3DFMT_A1R5G5B5;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A4R4G4B4)))
        {
            m_ddpfRGBA16[1] = D3DFMT_A4R4G4B4;
        }

        if (FAILED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
        {
            m_ddpfRGBA32[0] = m_ddpfRGBA16[0];
        }
        else
        {
            m_ddpfRGBA32[0] = D3DFMT_A8R8G8B8;
        }

        if (FAILED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
        {
            m_ddpfRGBA32[1] = m_ddpfRGBA16[1];
        }
        else
        {
            m_ddpfRGBA32[1] = D3DFMT_A8R8G8B8;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_P8)))
        {
            m_ddpfPalette[0] = D3DFMT_P8;
        }

        if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X8L8V8U8)))
        {
            m_ddpfEMBM[0] = D3DFMT_X8L8V8U8;
        }
        else if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_L6V5U5)))
        {
            m_ddpfEMBM[0] = D3DFMT_L6V5U5;
        }
        else if (SUCCEEDED(g_pd3dInterface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_V8U8)))
        {
            m_ddpfEMBM[0] = D3DFMT_V8U8;
        }
    }

    bool ZTextureManagerD3D::HasEMBM() const
    {
        return m_ddpfEMBM[0] != D3DFMT_UNKNOWN;
    }
    
    bool ZTextureManagerD3D::HasEMBMUVL() const
    {
        return m_ddpfEMBM[0] == D3DFMT_L6V5U5 || m_ddpfEMBM[0] == D3DFMT_X8L8V8U8;
    }

    bool ZTextureManagerD3D::HasDXT() const
    {
        return m_ddpfCompressed[0] != D3DFMT_UNKNOWN;
    }

    bool ZTextureManagerD3D::HasDXT_Nr(uint8_t lDxtNr) const
    {
        ZASSERT(lDxtNr);

        return m_ddpfCompressed[lDxtNr - 1] != D3DFMT_UNKNOWN;
    }

    IDirect3DCubeTexture9* ZTextureManagerD3D::CreateCubeTexture(const ZBitmap** ppBitmaps)
    {
        ZBitmap* pFirstBitmap = const_cast<ZBitmap*>(ppBitmaps[0]);

        const int lEdgeLength = pFirstBitmap->GetSizeX();
        for (int lFace = 0; lFace < 6; ++lFace)
        {
            ZBitmap* pFaceBitmap = const_cast<ZBitmap*>(ppBitmaps[lFace]);
            if (pFaceBitmap->GetSizeX() != lEdgeLength || pFaceBitmap->GetSizeY() != lEdgeLength)
            {
                return nullptr;
            }
        }

        const int lMipCount = pFirstBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }
        pFirstBitmap->GetMIPLevel(lStartMip); // called with discarded result in the PC build

        IDirect3DCubeTexture9* pCubeTexture = nullptr;
        m_pDev->CreateCubeTexture(lEdgeLength, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pCubeTexture, nullptr);

        for (int lFace = 0; lFace < 6; ++lFace)
        {
            ZBitmap* pFaceBitmap = const_cast<ZBitmap*>(ppBitmaps[lFace]);

            int lDstLevel = 0;
            for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
            {
                ZBitmap::TMipLevel* pMipLevel = pFaceBitmap->GetMIPLevel(lMip);

                D3DLOCKED_RECT lockedRect;
                pCubeTexture->LockRect(s_CubeFaces[lFace], lDstLevel, &lockedRect, nullptr, 0);

                ZBitmap::TDataBlock dataBlock;
                dataBlock.dwPosX = 0;
                dataBlock.dwPosY = 0;
                dataBlock.dwSizeX = pMipLevel->dwSizeX;
                dataBlock.dwSizeY = pMipLevel->dwSizeY;
                dataBlock.dwPich = lockedRect.Pitch;
                dataBlock.dwMaskR = 0xFF0000;
                dataBlock.dwMaskG = 0xFF00;
                dataBlock.dwMaskB = 0xFF;
                dataBlock.dwMaskA = 0xFF000000;
                dataBlock.dwBitCount = 32;
                dataBlock.bInvertAlpha = false;
                dataBlock.pData = lockedRect.pBits;

                pFaceBitmap->GetData(&dataBlock, lMip);

                pCubeTexture->UnlockRect(s_CubeFaces[lFace], lDstLevel);
            }
        }

        return pCubeTexture;
    }

    IDirect3DTexture9* ZTextureManagerD3D::CreateI8Texture(const ZBitmap* pBitmap)
    {
        ZBitmap* pSrcBitmap = const_cast<ZBitmap*>(pBitmap);

        const int lMipCount = pSrcBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }

        ZBitmap::TMipLevel* pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);

        IDirect3DTexture9* pTexture = nullptr;
        m_pDev->CreateTexture(pMipLevel->dwSizeX, pMipLevel->dwSizeY, lMipCount - lStartMip, 0, D3DFMT_L8, D3DPOOL_MANAGED, &pTexture, nullptr);

        int lDstLevel = 0;
        for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
        {
            pMipLevel = pSrcBitmap->GetMIPLevel(lMip);

            D3DLOCKED_RECT lockedRect;
            pTexture->LockRect(lDstLevel, &lockedRect, nullptr, 0);
            memcpy(lockedRect.pBits, pMipLevel->pData, pMipLevel->dwSize);
            pTexture->UnlockRect(lDstLevel);
        }

        return pTexture;
    }

    IDirect3DTexture9* ZTextureManagerD3D::CreateU8V8Texture(const ZBitmap* pBitmap)
    {
        ZBitmap* pSrcBitmap = const_cast<ZBitmap*>(pBitmap);

        const int lMipCount = pSrcBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }

        ZBitmap::TMipLevel* pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);

        IDirect3DTexture9* pTexture = nullptr;
        m_pDev->CreateTexture(pMipLevel->dwSizeX, pMipLevel->dwSizeY, lMipCount - lStartMip, 0, D3DFMT_A8L8, D3DPOOL_MANAGED, &pTexture, nullptr);
        if (!pTexture)
        {
            return nullptr;
        }

        int lDstLevel = 0;
        for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
        {
            pMipLevel = pSrcBitmap->GetMIPLevel(lMip);

            D3DLOCKED_RECT lockedRect;
            pTexture->LockRect(lDstLevel, &lockedRect, nullptr, 0);
            memcpy(lockedRect.pBits, pMipLevel->pData, pMipLevel->dwSize);
            pTexture->UnlockRect(lDstLevel);
        }

        return pTexture;
    }

    IDirect3DTexture9* ZTextureManagerD3D::CreateRGBATexture(const ZBitmap* pBitmap)
    {
        ZBitmap32 bitmap32Copy;
        ZBitmap* pSrcBitmap = const_cast<ZBitmap*>(pBitmap);

        D3DFORMAT format = m_ddpfRGBA32[0];
        int lBitCount = 32;
        const int lType = pSrcBitmap->GetType();
        if (lType == BITMAP_DXT1)
        {
            format = m_ddpfRGBA16[0];
            lBitCount = 16;
        }
        else if (lType == BITMAP_DXT3)
        {
            format = m_ddpfRGBA16[1];
            lBitCount = 16;
        }

        if (format == D3DFMT_UNKNOWN)
        {
            format = m_ddpfRGBA32[0];
            lBitCount = 32;
        }
        if (format == D3DFMT_A1R5G5B5 || format == D3DFMT_A4R4G4B4)
        {
            lBitCount = 16;
        }

        int lMipCount = pSrcBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }

        ZBitmap::TMipLevel* pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);
        if (static_cast<uint32_t>(pMipLevel->dwSizeX) > m_lMaxWidth || static_cast<uint32_t>(pMipLevel->dwSizeY) > m_lMaxHeight)
        {
            if (lMipCount == 1)
            {
                // Single-mip texture larger than the device limit: rebuild it with a full mip chain
                bitmap32Copy.Create(*pSrcBitmap, true);
                pSrcBitmap = &bitmap32Copy;
                lMipCount = pSrcBitmap->GetMipLevelCount();
            }

            while (true)
            {
                pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);
                if (static_cast<uint32_t>(pMipLevel->dwSizeX) <= m_lMaxWidth && static_cast<uint32_t>(pMipLevel->dwSizeY) <= m_lMaxHeight)
                {
                    break;
                }
                ++lStartMip;
            }
        }

        pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);

        IDirect3DTexture9* pTexture = nullptr;
        m_pDev->CreateTexture(pMipLevel->dwSizeX, pMipLevel->dwSizeY, lMipCount - lStartMip, 0, format, D3DPOOL_MANAGED, &pTexture, nullptr);

        ZBitmap::TDataBlock dataBlock;
        dataBlock.dwPosX = 0;
        dataBlock.dwPosY = 0;
        dataBlock.bInvertAlpha = false;
        if (lBitCount == 32)
        {
            dataBlock.dwMaskR = 0xFF0000;
            dataBlock.dwMaskG = 0xFF00;
            dataBlock.dwMaskB = 0xFF;
            dataBlock.dwMaskA = 0xFF000000;
            dataBlock.dwBitCount = 32;
        }
        else
        {
            dataBlock.dwBitCount = 16;
            if (format == D3DFMT_A1R5G5B5)
            {
                dataBlock.dwMaskA = 0x8000;
                dataBlock.dwMaskR = 0x7C00;
                dataBlock.dwMaskG = 0x3E0;
                dataBlock.dwMaskB = 0x1F;
            }
            else
            {
                dataBlock.dwMaskA = 0xF000;
                dataBlock.dwMaskR = 0xF00;
                dataBlock.dwMaskG = 0xF0;
                dataBlock.dwMaskB = 0xF;
            }
        }

        int lDstLevel = 0;
        for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
        {
            pMipLevel = pSrcBitmap->GetMIPLevel(lMip);

            D3DLOCKED_RECT lockedRect;
            pTexture->LockRect(lDstLevel, &lockedRect, nullptr, 0);

            dataBlock.dwSizeX = pMipLevel->dwSizeX;
            dataBlock.dwSizeY = pMipLevel->dwSizeY;
            dataBlock.dwPich = lockedRect.Pitch;
            dataBlock.pData = lockedRect.pBits;

            pSrcBitmap->GetData(&dataBlock, lMip);

            pTexture->UnlockRect(lDstLevel);
        }

        return pTexture;
    }

    IDirect3DTexture9* ZTextureManagerD3D::CreateDXTTexture(const ZBitmap* pBitmap)
    {
        ZBitmap* pSrcBitmap = const_cast<ZBitmap*>(pBitmap);

        D3DFORMAT format = D3DFMT_DXT1;
        if (pSrcBitmap->GetType() == BITMAP_DXT3)
        {
            format = D3DFMT_DXT3;
        }

        const int lMipCount = pSrcBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }

        ZBitmap::TMipLevel* pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);

        // DXT compression requires power-of-two dimensions
        uint32_t dwSizeX = pMipLevel->dwSizeX;
        uint32_t dwSizeY = pMipLevel->dwSizeY;
        uint32_t dwTest = dwSizeX;
        while ((dwTest & 1) == 0)
        {
            dwTest >>= 1;
        }
        if (dwTest != 1)
        {
            return nullptr;
        }
        dwTest = dwSizeY;
        while ((dwTest & 1) == 0)
        {
            dwTest >>= 1;
        }
        if (dwTest != 1)
        {
            return nullptr;
        }

        // ...and at least one 4x4 block
        if (dwSizeX < 4)
        {
            dwSizeX = 4;
        }
        if (dwSizeY < 4)
        {
            dwSizeY = 4;
        }

        IDirect3DTexture9* pTexture = nullptr;
        m_pDev->CreateTexture(dwSizeX, dwSizeY, lMipCount - lStartMip, 0, format, D3DPOOL_MANAGED, &pTexture, nullptr);

        int lDstLevel = 0;
        for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
        {
            pMipLevel = pSrcBitmap->GetMIPLevel(lMip);

            D3DLOCKED_RECT lockedRect;
            pTexture->LockRect(lDstLevel, &lockedRect, nullptr, 0);
            memcpy(lockedRect.pBits, pMipLevel->pData, pMipLevel->dwSize);
            pTexture->UnlockRect(lDstLevel);
        }

        return pTexture;
    }

    IDirect3DTexture9* ZTextureManagerD3D::CreateDOT3Texture(const ZBitmap* pBitmap)
    {
        ZBitmap* pSrcBitmap = const_cast<ZBitmap*>(pBitmap);

        const int lMipCount = pSrcBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }

        ZBitmap::TMipLevel* pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);

        IDirect3DTexture9* pTexture = nullptr;
        m_pDev->CreateTexture(pMipLevel->dwSizeX, pMipLevel->dwSizeY, lMipCount - lStartMip, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, nullptr);

        auto* pScratch = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * pSrcBitmap->GetSizeX() * pSrcBitmap->GetSizeY()));
        auto* pNormalMap = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * pSrcBitmap->GetSizeX() * pSrcBitmap->GetSizeY()));

        int lDstLevel = 0;
        for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
        {
            pMipLevel = pSrcBitmap->GetMIPLevel(lMip);

            D3DLOCKED_RECT lockedRect;
            pTexture->LockRect(lDstLevel, &lockedRect, nullptr, 0);

            ZBitmap::TDataBlock dataBlock;
            dataBlock.dwPosX = 0;
            dataBlock.dwPosY = 0;
            dataBlock.dwSizeX = pMipLevel->dwSizeX;
            dataBlock.dwSizeY = pMipLevel->dwSizeY;
            dataBlock.dwPich = 4 * pMipLevel->dwSizeX;
            dataBlock.dwMaskR = 0xFF0000;
            dataBlock.dwMaskG = 0xFF00;
            dataBlock.dwMaskB = 0xFF;
            dataBlock.dwMaskA = 0xFF000000;
            dataBlock.dwBitCount = 32;
            dataBlock.bInvertAlpha = false;
            dataBlock.pData = pScratch;

            pSrcBitmap->GetData(&dataBlock, lMip);

            const uint32_t dwRowBytes = 4 * pMipLevel->dwSizeX;
            MakeNormalMap(pNormalMap, pScratch, pMipLevel->dwSizeX, pMipLevel->dwSizeY, dwRowBytes, dwRowBytes);

            const uint8_t* pSrcRow = reinterpret_cast<const uint8_t*>(pNormalMap);
            uint8_t* pDstRow = static_cast<uint8_t*>(lockedRect.pBits);
            for (int lRow = 0; lRow < pMipLevel->dwSizeY; ++lRow)
            {
                memcpy(pDstRow, pSrcRow, dwRowBytes);
                pSrcRow += dwRowBytes;
                pDstRow += lockedRect.Pitch;
            }

            pTexture->UnlockRect(lDstLevel);
        }

        ZUniMemory::Free(pScratch);
        ZUniMemory::Free(pNormalMap);

        return pTexture;
    }

    IDirect3DTexture9* ZTextureManagerD3D::CreateDMAPTexture(const ZBitmap* pBitmap)
    {
        ZBitmap* pSrcBitmap = const_cast<ZBitmap*>(pBitmap);

        const int lMipCount = pSrcBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }

        ZBitmap::TMipLevel* pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);

        IDirect3DTexture9* pTexture = nullptr;
        m_pDev->CreateTexture(pMipLevel->dwSizeX, pMipLevel->dwSizeY, lMipCount - lStartMip, D3DUSAGE_DMAP, D3DFMT_L16, D3DPOOL_MANAGED, &pTexture, nullptr);

        auto* pScratch = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * pSrcBitmap->GetSizeX() * pSrcBitmap->GetSizeY()));
        auto* pDMap = static_cast<uint16_t*>(ZUniMemory::Allocate(2 * pSrcBitmap->GetSizeX() * pSrcBitmap->GetSizeY()));

        int lDstLevel = 0;
        for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
        {
            pMipLevel = pSrcBitmap->GetMIPLevel(lMip);

            D3DLOCKED_RECT lockedRect;
            pTexture->LockRect(lDstLevel, &lockedRect, nullptr, 0);

            ZBitmap::TDataBlock dataBlock;
            dataBlock.dwPosX = 0;
            dataBlock.dwPosY = 0;
            dataBlock.dwSizeX = pMipLevel->dwSizeX;
            dataBlock.dwSizeY = pMipLevel->dwSizeY;
            dataBlock.dwPich = 4 * pMipLevel->dwSizeX;
            dataBlock.dwMaskR = 0xFF0000;
            dataBlock.dwMaskG = 0xFF00;
            dataBlock.dwMaskB = 0xFF;
            dataBlock.dwMaskA = 0xFF000000;
            dataBlock.dwBitCount = 32;
            dataBlock.bInvertAlpha = false;
            dataBlock.pData = pScratch;

            pSrcBitmap->GetData(&dataBlock, lMip);

            const uint32_t dwRowBytes = 2 * pMipLevel->dwSizeX;
            MakeDMap(pDMap, pScratch, pMipLevel->dwSizeX, pMipLevel->dwSizeY, 4 * pMipLevel->dwSizeX, dwRowBytes);

            const uint8_t* pSrcRow = reinterpret_cast<const uint8_t*>(pDMap);
            uint8_t* pDstRow = static_cast<uint8_t*>(lockedRect.pBits);
            for (int lRow = 0; lRow < pMipLevel->dwSizeY; ++lRow)
            {
                memcpy(pDstRow, pSrcRow, dwRowBytes);
                pSrcRow += dwRowBytes;
                pDstRow += lockedRect.Pitch;
            }

            pTexture->UnlockRect(lDstLevel);
        }

        ZUniMemory::Free(pScratch);
        ZUniMemory::Free(pDMap);

        return pTexture;
    }

    IDirect3DTexture9* ZTextureManagerD3D::CreateEMBMTexture(const ZBitmap* pBitmap)
    {
        ZBitmap* pSrcBitmap = const_cast<ZBitmap*>(pBitmap);

        const D3DFORMAT format = m_ddpfEMBM[0];
        const int lMipCount = pSrcBitmap->GetMipLevelCount();
        int lStartMip = g_pSysInterface->m_lTextureResolution[0];
        if (lMipCount - 1 < lStartMip)
        {
            lStartMip = lMipCount - 1;
        }

        ZBitmap::TMipLevel* pMipLevel = pSrcBitmap->GetMIPLevel(lStartMip);

        IDirect3DTexture9* pTexture = nullptr;
        m_pDev->CreateTexture(pMipLevel->dwSizeX, pMipLevel->dwSizeY, lMipCount - lStartMip, 0, format, D3DPOOL_MANAGED, &pTexture, nullptr);

        auto* pScratch = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * pSrcBitmap->GetSizeX() * pSrcBitmap->GetSizeY()));
        auto* pBumpMap = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * pSrcBitmap->GetSizeX() * pSrcBitmap->GetSizeY()));

        int lDstLevel = 0;
        for (int lMip = lStartMip; lMip < lMipCount; ++lMip, ++lDstLevel)
        {
            pMipLevel = pSrcBitmap->GetMIPLevel(lMip);

            D3DLOCKED_RECT lockedRect;
            pTexture->LockRect(lDstLevel, &lockedRect, nullptr, 0);

            ZBitmap::TDataBlock dataBlock;
            dataBlock.dwPosX = 0;
            dataBlock.dwPosY = 0;
            dataBlock.dwSizeX = pMipLevel->dwSizeX;
            dataBlock.dwSizeY = pMipLevel->dwSizeY;
            dataBlock.dwPich = 4 * pMipLevel->dwSizeX;
            dataBlock.dwMaskR = 0xFF0000;
            dataBlock.dwMaskG = 0xFF00;
            dataBlock.dwMaskB = 0xFF;
            dataBlock.dwMaskA = 0xFF000000;
            dataBlock.dwBitCount = 32;
            dataBlock.bInvertAlpha = false;
            dataBlock.pData = pScratch;

            pSrcBitmap->GetData(&dataBlock, lMip);

            MakeBumpMap(pBumpMap, pScratch, pMipLevel->dwSizeX, pMipLevel->dwSizeY, format);

            const uint32_t dwRowBytes = (format == D3DFMT_X8L8V8U8 ? 4 : 2) * pMipLevel->dwSizeX;
            const uint8_t* pSrcRow = reinterpret_cast<const uint8_t*>(pBumpMap);
            uint8_t* pDstRow = static_cast<uint8_t*>(lockedRect.pBits);
            for (int lRow = 0; lRow < pMipLevel->dwSizeY; ++lRow)
            {
                memcpy(pDstRow, pSrcRow, dwRowBytes);
                pSrcRow += dwRowBytes;
                pDstRow += lockedRect.Pitch;
            }

            pTexture->UnlockRect(lDstLevel);
        }

        ZUniMemory::Free(pScratch);
        ZUniMemory::Free(pBumpMap);

        return pTexture;
    }

}