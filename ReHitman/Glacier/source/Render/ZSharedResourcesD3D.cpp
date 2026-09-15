#include <Glacier/Render/Material/ZRenderMaterialResourceD3DFX.h>
#include <Glacier/Render/PostFilter/ZPostFilter.h>
#include <Glacier/Render/Draw/ZRenderDrawD3D.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRD3DStaticVB.h>
#include <Glacier/Render/ZRD3DStaticIB.h>
#include <Glacier/Render/ZRD3DDynamicVB.h>
#include <Glacier/Render/ZRD3DDynamicIB.h>
#include <Glacier/Render/ZRD3DDynamicIB32.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZTextureManagerD3D.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/System/ZRX86AllocIf.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <cstdlib>
#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        #include "ZSharedResourcesD3D_ProceduralData.inc"

        void AllocateBigQuadVB()
        {
            constexpr DWORD kUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
            static_assert(kUsage == 0x208u);

            g_pd3dDevice->CreateVertexBuffer(0x70u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT1, nullptr);
            g_pd3dDevice->CreateVertexBuffer(0x90u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT2, nullptr);
            g_pd3dDevice->CreateVertexBuffer(0xB0u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT3, nullptr);
            g_pd3dDevice->CreateVertexBuffer(0xD0u, kUsage, 0, D3DPOOL_DEFAULT, &g_pVBPDT4, nullptr);
        }

        void FreeBigQuadVB()
        {
            if (g_pVBPDT1 && !g_pVBPDT1->Release())
            {
                g_pVBPDT1 = nullptr;
            }

            if (g_pVBPDT2 && !g_pVBPDT2->Release())
            {
                g_pVBPDT2 = nullptr;
            }

            if (g_pVBPDT3 && !g_pVBPDT3->Release())
            {
                g_pVBPDT3 = nullptr;
            }

            if (g_pVBPDT4 && !g_pVBPDT4->Release())
            {
                g_pVBPDT4 = nullptr;
            }
        }

        // PC keeps the raw D3D interface pointer in the first dword of these texture slots
        // (overlapping ZTextureBase::m_usSize), e.g. the spot-attenuation maps and the shadow
        // colour maps - ZSharedResourcesD3D::Allocate/Free and the binder all agree on offset 0.
        IDirect3DTexture9* GetSlotTexture(ZTextureD3D& rSlot)
        {
            return static_cast<IDirect3DTexture9*>(*reinterpret_cast<void**>(&rSlot.m_usSize));
        }

        void StoreSlotTexture(ZTextureD3D& rSlot, IDirect3DTexture9* pTexture)
        {
            *reinterpret_cast<void**>(&rSlot.m_usSize) = pTexture;
        }

        void ReleaseSlotTexture(ZTextureD3D& rSlot)
        {
            IDirect3DTexture9* pTexture = GetSlotTexture(rSlot);
            if (pTexture)
            {
                pTexture->Release();
                StoreSlotTexture(rSlot, nullptr);
            }
        }

        // ---------------------------------------------------------------------
        // Procedural light/phase map generators (PC 0x0048C0F0 - 0x0048EAC1).
        // Each routine fills every mip level of the (square) texture with a
        // normalized [-1,1] parametric pattern; formulas transcribed from the
        // scalar x87 disassembly exported by Tools/ida_dump.
        // ---------------------------------------------------------------------

        double ClampUnit(double v)
        {
            if (v < 0.0) return 0.0;
            if (v > 1.0) return 1.0;
            return v;
        }

        uint32_t RoundByte(double v)
        {
            const int i = static_cast<int>(v + 0.5);
            return i > 255 ? 255u : static_cast<uint32_t>(i > 0 ? i : 0);
        }

        uint32_t TruncByte(double v)
        {
            const int i = static_cast<int>(v);
            return i > 255 ? 255u : static_cast<uint32_t>(i > 0 ? i : 0);
        }

        uint32_t PackRGB(uint32_t lR, uint32_t lG, uint32_t lB)
        {
            return 0xFF000000u | (lR << 16) | (lG << 8) | lB;
        }

        template <typename Fn>
        void Fill2DMap(IDirect3DTexture9* pTexture, Fn fn)
        {
            const uint32_t lLevels = pTexture->GetLevelCount();
            for (uint32_t lLevel = 0; lLevel < lLevels; ++lLevel)
            {
                D3DSURFACE_DESC Desc;
                pTexture->GetLevelDesc(lLevel, &Desc);
                D3DLOCKED_RECT LockedRect;
                pTexture->LockRect(lLevel, &LockedRect, nullptr, 0);
                for (int y = 0; y < static_cast<int>(Desc.Height); ++y)
                {
                    auto* pRow = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(LockedRect.pBits) + static_cast<size_t>(y) * LockedRect.Pitch);
                    for (int x = 0; x < static_cast<int>(Desc.Width); ++x)
                    {
                        pRow[x] = fn(x, y, static_cast<int>(Desc.Width), static_cast<int>(Desc.Height));
                    }
                }
                pTexture->UnlockRect(lLevel);
            }
        }

        // PC sub_48C0F0: 1 - exp(-0.5 * (1/|u| + 1/|v|)); axis hits are opaque black.
        uint32_t ScatterPixel(int x, int y, int w, int h)
        {
            const double u = 2.0 * (x + 0.5) / w - 1.0;
            const double v = 2.0 * (y + 0.5) / h - 1.0;
            if (u == 0.0 || v == 0.0)
            {
                return 0xFF000000u;
            }
            const double au = std::fabs(u);
            const double av = std::fabs(v);
            const double fValue = 1.0 - std::exp(-(1.0 / au + 1.0 / av) * 0.5);
            const double d = au + av;
            return PackRGB(
                RoundByte(fValue * 0.8 / d * 63.75),
                RoundByte(fValue * 0.5 / d * 63.75),
                RoundByte(fValue * 0.4 / d * 63.75));
        }

        void CreateScatterMap(IDirect3DTexture9* pTexture)
        {
            Fill2DMap(pTexture, &ScatterPixel);
        }

        // PC sub_48DCE0: pow(cos(row angle), col power), all four bytes replicated.
        uint32_t SpecularPixel(int x, int y, int w, int h)
        {
            const double fAngle = (y + 0.5) / h * 1.5707964;
            const double fPower = (x + 0.5) / w * 128.0;
            const double fColor = std::pow(std::cos(fAngle), fPower);
            const uint32_t lV = RoundByte(fColor * 255.0);
            return lV | (lV << 8) | (lV << 16) | (lV << 24);
        }

        void CreateSpecularMap(IDirect3DTexture9* pTexture)
        {
            Fill2DMap(pTexture, &SpecularPixel);
        }

        // PC sub_48D7E0: A = pow(dot,128), RGB = sqrt(1 - col^2).
        uint32_t AnisotropicPixel(int x, int y, int w, int h)
        {
            const double fCol = 2.0 * (x + 0.5) / w - 1.0;
            const double fRow = 2.0 * (y + 0.5) / h - 1.0;
            const double fDot = fCol * fRow + std::sqrt(1.0 - fCol * fCol) * std::sqrt(1.0 - fRow * fRow);
            const uint32_t lA = RoundByte(std::pow(fDot, 128.0) * 255.0);
            const uint32_t lV = RoundByte(std::sqrt(1.0 - fCol * fCol) * 255.0);
            return (lA << 24) | (lV << 16) | (lV << 8) | lV;
        }

        void CreateAnisotropicMap(IDirect3DTexture9* pTexture, float)
        {
            Fill2DMap(pTexture, &AnisotropicPixel);
        }

        // PC sub_48DEB0: Henyey-Greenstein (1-g^2)/(1+g^2-2gu)^1.5, g = {0.8, 0.3, 0}.
        uint32_t HgPhasePixel(int x, int y, int w, int h)
        {
            const double u = 2.0 * (y + 0.5) / h - 1.0;
            const auto fHG = [u](double fG) {
                const double fNum = 1.0 - fG * fG;
                const double fDen = fG * fG + 1.0 - 2.0 * fG * u;
                return fNum / std::pow(fDen, 1.5);
            };
            return PackRGB(
                TruncByte(fHG(0.8) * 63.75),
                TruncByte(fHG(0.3) * 63.75),
                TruncByte(fHG(0.0) * 63.75));
        }

        void CreatehgPhaseMap(IDirect3DTexture9* pTexture)
        {
            Fill2DMap(pTexture, &HgPhasePixel);
        }

        // PC sub_48E0E0: clamp to [lo,hi], then smoothstep 3t^2 - 2t^3.
        double SmoothStepLoHi(double v, double fLo, double fHi)
        {
            if (v < fLo)
            {
                v = fLo;
            }
            else if (v > fHi)
            {
                v = fHi;
            }
            const double t = (v - fLo) / (fHi - fLo);
            return 3.0 * t * t - 2.0 * t * t * t;
        }

        // PC sub_48E150 (Fresnel-style). Exact byte packing reproduced from the disassembly.
        uint32_t RefractionPixel(int x, int y, int w, int h)
        {
            const double fU = 2.0 * (x + 0.5) / w - 1.0;
            if (fU == 0.0)
            {
                return 0u;
            }
            constexpr double kK = 0.5102041059610798;
            constexpr double kC = 0.7142857313156128;
            const double fNeg = -fU;
            const double fT = 1.0 - (1.0 - fU * fU) * kK;
            const double fS = fT < 0.0 ? 0.0 : 1.0;
            const double fRt = fT > 0.0 ? std::sqrt(fT) : 0.0;
            const double fC = kC * fS;
            const double fM = (fNeg * kC - fRt) * fS;
            const double fD = -(fC * fU + fM);
            const double fE = fD != 0.0 ? fNeg / fD : 0.0;
            const double fP = fNeg != 0.0 ? fD / fNeg : 0.0;
            const double fA = (fE - kC) / (fE + kC);
            const double fB = (fP - kC) / (fP + kC);
            const double fH = (fA * fA + fB * fB) * 0.5 * fS + (1.0 - fS);
            const double fR = ClampUnit(fC);
            const double fG = ClampUnit(-fM);
            const double fSm = SmoothStepLoHi(ClampUnit(fH), 0.0, 0.5);

            const int32_t lA = static_cast<int32_t>((1.0 - fSm) * -255.0);
            const int32_t lX = static_cast<int32_t>(fR * -255.0);
            const int32_t lZ = static_cast<int32_t>(fG * -255.0);
            int64_t lResult = (0xFF00LL - lX) << 8;
            lResult = ((lResult - lZ) << 8) - lA;
            return static_cast<uint32_t>(lResult);
        }

        void CreateRefractionMap(IDirect3DTexture9* pTexture)
        {
            Fill2DMap(pTexture, &RefractionPixel);
        }

        // PC CreateNormalizerMap 0x0048E580 + sub_48E480: each cube texel stores the
        // normalized direction to the texel center, encoded as n*0.5+0.5 per axis.
        void CreateNormalizerMap(IDirect3DCubeTexture9* pCube)
        {
            for (uint32_t lFace = 0; lFace < 6; ++lFace)
            {
                const uint32_t lLevels = pCube->GetLevelCount();
                for (uint32_t lLevel = 0; lLevel < lLevels; ++lLevel)
                {
                    D3DSURFACE_DESC Desc;
                    pCube->GetLevelDesc(lLevel, &Desc);
                    const int lDim = static_cast<int>(Desc.Width);

                    D3DLOCKED_RECT LockedRect;
                    pCube->LockRect(static_cast<D3DCUBEMAP_FACES>(lFace), lLevel, &LockedRect, nullptr, 0);

                    for (int y = 0; y < lDim; ++y)
                    {
                        auto* pRow = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(LockedRect.pBits) + static_cast<size_t>(y) * LockedRect.Pitch);
                        const double fRow = 2.0 * (y + 0.5) / lDim - 1.0;
                        for (int x = 0; x < lDim; ++x)
                        {
                            const double fCol = 2.0 * (x + 0.5) / lDim - 1.0;
                            double fD[3];
                            switch (lFace)
                            {
                            case 0: fD[0] = 1.0; fD[1] = -fRow; fD[2] = -fCol; break;
                            case 1: fD[0] = -1.0; fD[1] = -fRow; fD[2] = fCol; break;
                            case 2: fD[0] = fCol; fD[1] = 1.0; fD[2] = fRow; break;
                            case 3: fD[0] = fCol; fD[1] = -1.0; fD[2] = -fRow; break;
                            case 4: fD[0] = fCol; fD[1] = -fRow; fD[2] = 1.0; break;
                            default: fD[0] = -fCol; fD[1] = -fRow; fD[2] = -1.0; break;
                            }

                            const double fLen = std::sqrt(fD[0] * fD[0] + fD[1] * fD[1] + fD[2] * fD[2]);
                            pRow[x] = PackRGB(
                                RoundByte((fD[0] / fLen * 0.5 + 0.5) * 255.0),
                                RoundByte((fD[1] / fLen * 0.5 + 0.5) * 255.0),
                                RoundByte((fD[2] / fLen * 0.5 + 0.5) * 255.0));
                        }
                    }
                    pCube->UnlockRect(static_cast<D3DCUBEMAP_FACES>(lFace), lLevel);
                }
            }
        }

        // PC ZSharedResourcesD3D::Allocate 0x0048A452-0x0048A711: registers the four
        // container textures (Mouse16x16, Font8x13, ShadowClip1x2, White1x1) whose D3D
        // objects later live in the wrappers' m_pUserData (released again in Free()).
        void RegisterProceduralBitmap(ZTextureManagerD3D* pTexCon, const uint32_t* pData, int lSizeX, int lSizeY, const char* pszName, ZTextureD3D& rSlot)
        {
            ZBitmap32 bitmap(const_cast<uint32_t*>(pData), lSizeX, lSizeY, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000, false);
            bitmap.SetScaleFactor(0.0f);
            bitmap.SetName(pszName);
            pTexCon->CreateTexture(&bitmap, &rSlot);
        }

        // PC Allocate 0x0048A50C/0x0048A52C fills one float pair per ASCII code:
        // u = (8*(c & 0xF)) / 128, v = (13*(c / 16)) / 128 (PC flt_8EE130 / flt_8EE134).
        void FillFontLetterPositions(ZFontLetterPositionTable& rTable)
        {
            for (int lChar = 0x20; lChar < 0x7F; ++lChar)
            {
                rTable.m_fUV[lChar][0] = static_cast<float>(8 * (lChar & 0xF)) * (1.0f / 128.0f);
                rTable.m_fUV[lChar][1] = static_cast<float>(13 * (lChar / 16)) * (1.0f / 128.0f);
            }
        }

        void CreateProceduralTextures(ZTextureManagerD3D* pTexCon)
        {
            FillFontLetterPositions(g_FontLetterPositions);

            RegisterProceduralBitmap(pTexCon, kProceduralMouseData, 16, 16, "Mouse16x16", g_texMouse16x16);

            auto* pFontPixels = static_cast<uint32_t*>(ZUniMemory::Allocate(sizeof(uint32_t) * 128 * 128));
            std::memset(pFontPixels, 0, sizeof(uint32_t) * 128 * 128);

            for (int lChar = 0x20; lChar < 0x7F; ++lChar)
            {
                const int lX0 = 8 * (lChar & 0xF);
                const int lY0 = 13 * (lChar / 16);
                const uint8_t* pGlyph = kProceduralFontData + (lChar - 0x20) * 13;
                for (int lRow = 0; lRow < 13; ++lRow)
                {
                    const uint8_t lBits = pGlyph[lRow];
                    for (int lCol = 0; lCol < 8; ++lCol)
                    {
                        pFontPixels[(lY0 + lRow) * 128 + (lX0 + lCol)] = (lBits & (0x80u >> lCol)) ? 0xFFFFFFFFu : 0u;
                    }
                }
            }

            RegisterProceduralBitmap(pTexCon, pFontPixels, 128, 128, "Font8x13", g_texFont8x13);
            ZUniMemory::Free(pFontPixels);

            const uint32_t aShadowClip[2] = { 0u, 0xFFFFFFFFu };
            RegisterProceduralBitmap(pTexCon, aShadowClip, 2, 1, "ShadowClip1x2", g_texShadowClip1x2);

            const uint32_t lWhitePixel = 0xFFFFFFFFu;
            RegisterProceduralBitmap(pTexCon, &lWhitePixel, 1, 1, "White1x1", g_texWhite);
        }

        // PC CreateSpotAttenuationMap 0x0048D8D0. mode 0 = radial falloff, mode 1 = per-axis.
        void CreateSpotAttenuationMap(IDirect3DTexture9* pTexture, float fRatio, int lMode)
        {
            Fill2DMap(
                pTexture,
                [fRatio, lMode](int x, int y, int w, int h) -> uint32_t {
                    const double fCol = 2.0 * (x + 0.5) / w - 1.0;
                    const double fRow = 2.0 * (y + 0.5) / h - 1.0;

                    double fValue = 0.0;
                    const double fRatioD = fRatio;
                    if (lMode != 0)
                    {
                        const double fX = ClampUnit((std::fabs(fCol) - fRatioD) / (1.0 - fRatioD));
                        const double fY = ClampUnit((std::fabs(fRow) - fRatioD) / (1.0 - fRatioD));
                        const double fBase = (1.0 - fX) * (1.0 - fY);
                        fValue = fBase * fBase * (3.0 - 2.0 * fBase);
                    }
                    else
                    {
                        const double fDist = std::sqrt(fCol * fCol + fRow * fRow);
                        if (fDist < 1.0)
                        {
                            if (fDist <= fRatioD)
                            {
                                fValue = 1.0;
                            }
                            else
                            {
                                const double fT = 1.0 - (fDist - fRatioD) / (1.0 - fRatioD);
                                fValue = fT * fT * (3.0 - 2.0 * fT);
                            }
                        }
                    }

                    if (!y || y == h - 1 || !x || x == w - 1)
                    {
                        fValue = 0.0;
                    }

                    const uint32_t lV = RoundByte(fValue * 255.0);
                    return lV | (lV << 8) | (lV << 16) | (lV << 24);
                });
        }
    }

    void ZSharedResourcesD3D::Create()
    {
        ZASSERT(!ZSharedResourcesD3D::g_pInstance);

        ZSharedResourcesD3D::g_pInstance = ZUniMemory::New<ZSharedResourcesD3D>();
    }

    void ZSharedResourcesD3D::Release()
    {
        ZASSERT(ZSharedResourcesD3D::g_pInstance);
        ZSharedResourcesD3D::g_pInstance->Free();

        ZUniMemory::Delete(ZSharedResourcesD3D::g_pInstance);
        ZSharedResourcesD3D::g_pInstance = nullptr;
    }

    ZSharedResourcesD3D::ZSharedResourcesD3D()
    {
        // PC 0x00487E90. Members that the original relied on the (zeroed) allocator to clear
        // (m_pVDNull, ...) are initialised here to keep the runtime state sane.
        m_pVDNull = nullptr;
        m_pSVB = nullptr;
        m_pSIB = nullptr;
        m_pVertexAllocator = nullptr;
        m_pIndexAllocator = nullptr;
        m_pDVB = nullptr;
        m_pDIB = nullptr;
        m_pDIB32 = nullptr;
        m_bUseVertexShader = false;
        m_bUsePixelShader = false;
        m_bDebugVertexShader = false;
        m_bDebugPixelShader = false;
        m_bUseSoftwareVertexProcessing = false;
        m_bUsePartialPrecisionShader = false;
        m_bBlurDropShadow = false;
        m_iShaderResolution = 256;
        m_eShadowMode = SHADOWMODE_COLOR;
        m_lShadowColorFormat = D3DFMT_A8R8G8B8;
        m_lShadowMapPassesNr = 0;
        m_bAllocated = false;

        m_pRenderDrawShared = ZUniMemory::New<ZRenderDrawD3D>();
        m_pResourceFX = ZUniMemory::New<ZRenderMaterialResourceD3DFX>();

        m_bBlurDropShadow = true;
        if (ZSysInterface::GetOption("DisableDropShadowBlur", nullptr))
        {
            m_bBlurDropShadow = false;
        }

        // Reads a value left over from a previous instance (the allocator is not zeroing);
        // only ever set through ZRenderWintelD3D::Init after construction.
        if (m_lShaderQuality >= SHADERQUALITY_HIGH)
        {
            m_iShaderResolution = 512;
        }

        char* pszShadowResolution = nullptr;
        if (ZSysInterface::GetOption("ShadowResolution", &pszShadowResolution))
        {
            m_iShaderResolution = std::atol(pszShadowResolution);
        }

        if (ZSysInterface::GetOption("RenderUsePartialPrecisionShader", nullptr))
        {
            m_bUsePartialPrecisionShader = true;
        }
    }

    ZSharedResourcesD3D::~ZSharedResourcesD3D()
    {
        // PC 0x0048C010
        Free();

        ZUniMemory::Delete(m_pResourceFX);
        m_pResourceFX = nullptr;

        if (m_pRenderDrawShared)
        {
            m_pRenderDrawShared->End();
            ZUniMemory::Delete(m_pRenderDrawShared);
            m_pRenderDrawShared = nullptr;
        }
    }

    void ZSharedResourcesD3D::Free()
    {
        if (m_bAllocated)
        {
            m_bAllocated = false;

            if (m_pVertexAllocator)
            {
                ZUniMemory::Delete(m_pVertexAllocator);
                m_pVertexAllocator = nullptr;
            }

            if (m_pIndexAllocator)
            {
                ZUniMemory::Delete(m_pIndexAllocator);
                m_pIndexAllocator = nullptr;
            }

            if (m_pSVB)
            {
                ZUniMemory::Delete(m_pSVB);
                m_pSVB = nullptr;
            }

            if (m_pSIB)
            {
                ZUniMemory::Delete(m_pSIB);
                m_pSIB = nullptr;
            }

            if (m_pDVB)
            {
                ZUniMemory::Delete(m_pDVB);
                m_pDVB = nullptr;
            }

            if (m_pDIB)
            {
                ZUniMemory::Delete(m_pDIB);
                m_pDIB = nullptr;
            }

            if (m_pDIB32)
            {
                ZUniMemory::Delete(m_pDIB32);
                m_pDIB32 = nullptr;
            }

            m_pResourceFX->FreeResources();

            if (m_pVDNull && !m_pVDNull->Release())
            {
                m_pVDNull = nullptr;
            }

            // The four procedural textures registered with the texture container
            // (White1x1 -> g_texWhite, Mouse16x16, Font8x13, ShadowClip1x2). PC keeps their
            // D3D objects in m_pUserData; dword_90ADF4 / dword_90AE3C / dword_90AE84 /
            // dword_90AECC.
            if (g_texWhite.m_pUserData)
            {
                g_texWhite.UserData<IDirect3DTexture9>()->Release();
                g_texWhite.m_pUserData = nullptr;
            }

            if (g_texMouse16x16.m_pUserData)
            {
                g_texMouse16x16.UserData<IDirect3DTexture9>()->Release();
                g_texMouse16x16.m_pUserData = nullptr;
            }

            if (g_texFont8x13.m_pUserData)
            {
                g_texFont8x13.UserData<IDirect3DTexture9>()->Release();
                g_texFont8x13.m_pUserData = nullptr;
            }

            if (g_texShadowClip1x2.m_pUserData)
            {
                g_texShadowClip1x2.UserData<IDirect3DBaseTexture9>()->Release();
                g_texShadowClip1x2.m_pUserData = nullptr;
            }

            if (g_pNormalizedCubeMap)
            {
                g_pNormalizedCubeMap->Release();
                g_pNormalizedCubeMap = nullptr;
            }

            if (g_pAnisotropicMap)
            {
                g_pAnisotropicMap->Release();
                g_pAnisotropicMap = nullptr;
            }

            if (g_pSpecularMap)
            {
                g_pSpecularMap->Release();
                g_pSpecularMap = nullptr;
            }

            if (g_pScatterMap)
            {
                g_pScatterMap->Release();
                g_pScatterMap = nullptr;
            }

            if (g_pghPhaseMap)
            {
                g_pghPhaseMap->Release();
                g_pghPhaseMap = nullptr;
            }

            if (g_pRefractionMap)
            {
                g_pRefractionMap->Release();
                g_pRefractionMap = nullptr;
            }

            for (int i = 0; i < 2; ++i)
            {
                ReleaseSlotTexture(g_pSpotAttenuationMap[i]);
                ReleaseSlotTexture(g_pSpotAttenuationMap2[i]);
            }

            for (int lPassId = 0; lPassId < m_lShadowMapPassesNr; ++lPassId)
            {
                ReleaseSlotTexture(g_texShadowMapColor[lPassId]);
            }

            if (g_pShadowMapDepthBuffers)
            {
                g_pShadowMapDepthBuffers->Release();
                g_pShadowMapDepthBuffers = nullptr;
            }

            for (int i = 0; i < MAX_ENV_TEXTURES_NR; ++i)
            {
                for (int j = 0; j < MAX_ENV_SURFACES_NR; ++j)
                {
                    m_pEnvSurface[i][j]->Release();
                }

                m_pEnvTextures[i]->Release();
            }

            if (m_pEnvDepth)
            {
                m_pEnvDepth->Release();
                m_pEnvDepth = nullptr;
            }

            FreeBigQuadVB();

            if (g_pPostFilter)
            {
                g_pPostFilter->FreeDeviceBuffers();
            }

            if (m_bBlurDropShadow)
            {
                m_BlurTexture.FreeDeviceBuffers();
            }
        }
    }

    void ZSharedResourcesD3D::Allocate()
    {
        // PC 0x0048A160
        if (m_bAllocated)
        {
            return;
        }

        m_bAllocated = true;

        // Vertex/index buffer usage flags. The "software vertex processing" flavour adds
        // D3DUSAGE_DONOTCLIP (and drops D3DUSAGE_SOFTWAREPROCESSING) as in the PC binary.
        uint32_t lSVBUsage = D3DUSAGE_WRITEONLY;                                             // 0x8
        uint32_t lSIBUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING;               // 0x28
        uint32_t lDVBUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;                          // 0x208
        uint32_t lDIBUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING; // 0x228
        if (m_bUseSoftwareVertexProcessing)
        {
            lSVBUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP;                             // 0x18
            lSIBUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP;                             // 0x18
            lDVBUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP;          // 0x218
            lDIBUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP;          // 0x218
        }

        const uint32_t lNumVertices = (ZSysInterface::GetOption("ExpandVertexBuffers", nullptr) ? 32u : 12u) << 20;

        m_pVertexAllocator = ZUniMemory::New<ZRX86AllocIf>(lNumVertices);
        m_pIndexAllocator = ZUniMemory::New<ZRX86AllocIf>(0x200000);

        const D3DPOOL ePool = g_lOverrideSLI ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT;
        m_pSVB = ZUniMemory::New<ZRD3DStaticVB>(g_pd3dDevice, lNumVertices, lSVBUsage, ePool);
        m_pSIB = ZUniMemory::New<ZRD3DStaticIB>(g_pd3dDevice, 0x400000, lSIBUsage, ePool);
        m_pDVB = ZUniMemory::New<ZRD3DDynamicVB>(g_pd3dDevice, 0x80000, lDVBUsage, D3DPOOL_DEFAULT);
        m_pDIB = ZUniMemory::New<ZRD3DDynamicIB>(g_pd3dDevice, 0x8000, lDIBUsage, D3DPOOL_DEFAULT);
        m_pDIB32 = ZUniMemory::New<ZRD3DDynamicIB32>(g_pd3dDevice, 0x20000, lDIBUsage, D3DPOOL_DEFAULT);

        ZSharedResourcesD3D::g_bSVBLockAcquired = true;

        // Dummy vertex declaration (FLOAT3 position stream + UNUSED terminator).
        D3DVERTEXELEMENT9 VertexElements[2] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0 },
        };
        g_pd3dDevice->CreateVertexDeclaration(VertexElements, &m_pVDNull);

        // Container textures Mouse16x16 / Font8x13 / ShadowClip1x2 / White1x1
        // (PC 0x0048A452 - 0x0048A711). The per-glyph texel-position tables at
        // flt_8EE130/flt_8EE134 (used only by the yet-unreversed fixed-font writer)
        // are intentionally not written here yet.
        if (auto* pTexCon = static_cast<ZTextureManagerD3D*>(g_pRenderDll->m_pTexCon))
        {
            CreateProceduralTextures(pTexCon);
        }

        // Procedural light/phase maps (PC 0x0048A711 - 0x0048A8F0).
        IDirect3DCubeTexture9* pNormalizerCube = nullptr;
        g_pd3dDevice->CreateCubeTexture(0x80, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pNormalizerCube, nullptr);
        CreateNormalizerMap(pNormalizerCube);
        g_pNormalizedCubeMap = pNormalizerCube;

        const auto CreateMapTexture = [](uint32_t lSize) -> IDirect3DTexture9* {
            IDirect3DTexture9* pTexture = nullptr;
            g_pd3dDevice->CreateTexture(lSize, lSize, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, nullptr);
            return pTexture;
        };

        IDirect3DTexture9* pMap = CreateMapTexture(0x80);
        CreateAnisotropicMap(pMap, 40.0f);
        g_pAnisotropicMap = pMap;

        pMap = CreateMapTexture(0x80);
        CreateSpecularMap(pMap);
        g_pSpecularMap = pMap;

        pMap = CreateMapTexture(0x80);
        CreateScatterMap(pMap);
        g_pScatterMap = pMap;

        pMap = CreateMapTexture(0x80);
        CreatehgPhaseMap(pMap);
        g_pghPhaseMap = pMap;

        pMap = CreateMapTexture(0x80);
        CreateRefractionMap(pMap);
        g_pRefractionMap = pMap;

        const float kSpotRatios[2] = { 0.25f, 0.75f };
        for (int lSpotIndex = 0; lSpotIndex < 2; ++lSpotIndex)
        {
            IDirect3DTexture9* pSpot = CreateMapTexture(0x20);
            CreateSpotAttenuationMap(pSpot, kSpotRatios[lSpotIndex], 0);
            StoreSlotTexture(g_pSpotAttenuationMap[lSpotIndex], pSpot);

            pSpot = CreateMapTexture(0x20);
            CreateSpotAttenuationMap(pSpot, kSpotRatios[lSpotIndex], 1);
            StoreSlotTexture(g_pSpotAttenuationMap2[lSpotIndex], pSpot);
        }

        // Shadow maps (colour mode only; the other modes drive their own render targets).
        if (g_pSysInterface->m_lShadowDetail)
        {
            m_lShadowMapPassesNr = 4;
            if (m_iShaderResolution >= 0x200)
                m_lShadowMapPassesNr = 2;
            if (m_iShaderResolution >= 0x400)
                m_lShadowMapPassesNr = 1;

            if (m_eShadowMode == SHADOWMODE_COLOR)
            {
                for (int lPassId = 0; lPassId < m_lShadowMapPassesNr; ++lPassId)
                {
                    IDirect3DTexture9* pColorMap = nullptr;
                    g_pd3dDevice->CreateTexture(m_iShaderResolution, m_iShaderResolution, 1, D3DUSAGE_RENDERTARGET, m_lShadowColorFormat, D3DPOOL_DEFAULT, &pColorMap, nullptr);
                    StoreSlotTexture(g_texShadowMapColor[lPassId], pColorMap);
                }

                g_pd3dDevice->CreateDepthStencilSurface(m_iShaderResolution, m_iShaderResolution, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, true, &g_pShadowMapDepthBuffers, nullptr);
            }
        }

        // Environment cube-map render targets (64x64) + the shared env depth/stencil surface.
        for (int i = 0; i < MAX_ENV_TEXTURES_NR; ++i)
        {
            IDirect3DCubeTexture9* pEnvCubeMap = nullptr;
            g_pd3dDevice->CreateCubeTexture(0x40, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pEnvCubeMap, nullptr);
            m_pEnvTextures[i] = pEnvCubeMap;

            for (int j = 0; j < MAX_ENV_SURFACES_NR; ++j)
            {
                pEnvCubeMap->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(j), 0, &m_pEnvSurface[i][j]);
            }
        }

        g_pd3dDevice->CreateDepthStencilSurface(0x40, 0x40, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &m_pEnvDepth, nullptr);

        AllocateBigQuadVB();

        if (g_pPostFilter)
        {
            g_pPostFilter->AllocateDeviceBuffers();
        }

        if (m_bBlurDropShadow)
        {
            m_BlurTexture.Init(m_iShaderResolution, m_iShaderResolution);
            m_BlurTexture.AllocateDeviceBuffers();
        }
    }

    IDirect3DTexture9* ZSharedResourcesD3D::BlurTexture(IDirect3DTexture9* pTexture, float fBlur1, float fBlur2, int lFlags, bool bSinglePass)
    {
        // iOS 0x100247924 / Xenon 0x82167A08 wrap the blur the same way: save the current
        // render targets, run the blur chain, restore the targets. On the PC build this wrapper
        // is inlined into ZRenderDrawD3D::Update (0x4B0AF0, call at 0x4B1CEC); a single
        // ZBlurTextureWintelD3D member is used instead of the iOS blur-slot vector.
        m_BlurTexture.StoreRenderTargets();
        IDirect3DTexture9* pResult = m_BlurTexture.Blur(pTexture, fBlur1, fBlur2, lFlags, bSinglePass);
        m_BlurTexture.RestoreRenderTargets();
        return pResult;
    }

    void ZSharedResourcesD3D::LockBuffersForCopy()
    {
        // iOS locks the current and next copies of its triple-buffered
        // static VB/IB sets; the PC layout has a single SVB/SIB pair.
        m_pSVB->LockedData();
        m_pSIB->LockedData();
    }

    STATIC_CLASS_VAR_IMPL(ZSharedResourcesD3D, ZSharedResourcesD3D*, g_pInstance, 0x0090AF20, nullptr);
    STATIC_CLASS_VAR_IMPL(ZSharedResourcesD3D, bool, g_bSVBLockAcquired, 0x0090AEF1, false);
}
