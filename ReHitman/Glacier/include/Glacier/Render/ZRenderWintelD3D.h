#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderWintel.h>
#include <Glacier/Render/D3D9.h>


namespace Glacier
{
    class ZCAMERA;

    // PC vtable 0x007638EC (160 slots). The Direct3D9 render implementation. Owns the D3D device,
    // swap chain, offscreen/antialias surfaces, Bink video texture and the present parameters.
    class ZRenderWintelD3D : public ZRenderWintel
    {
    public:
        // vtbl (overrides)
        ~ZRenderWintelD3D() override;
        void ColorFill() override; // slot 9 (PC 0x00488BE0) - fills the colour/antialias surfaces
        bool BeginScene() override; // PC 0x00488C20
        bool EndScene() override; // PC 0x00488CB0
        void Flip() override; // implements the base pure virtual (PC 0x0048BE70)
        void Dump(const char* pName) override; // PC 0x00489430
        void SetViewport(const float, const float, const float, const float) override; // PC 0x00488CF0
        void ClearViewport(const unsigned int, const unsigned int) override; // PC 0x00488EA0
        void SetProjection(const float, const float, const float, const float, const float, const float, const bool) override; // PC 0x00489160
        void SetModelView(const float* pMatrix, const float* pPos, const float, const float) override; // PC 0x0042A470
        void SetFog(const float, const float, const unsigned int) override; // PC 0x00489170
        const char* GetRenderName() override; // PC 0x00488410
        void ProgressBar(const float, const unsigned int, const unsigned int) override; // PC 0x0048ABB0
        void GetRenderCaps(_GLC_RENDERCAPS* pRenderCaps) override; // PC 0x004888E0
        void SetRenderState(const _GLC_RENDERSTATE eRenderState, const unsigned int* pValue) override; // PC 0x004889C0
        void SetGammaValue(const float fGamma) override; // PC 0x00489370
        unsigned int ShadowRenderLayerId() override; // PC 0x00489C30
        bool ShadowRenderStart(unsigned int) override; // PC 0x00489C50
        unsigned int NumShadowRenderCasterPasses(unsigned int) override; // PC 0x00489C60
        unsigned int ShadowRenderCasterStart(unsigned int, unsigned int) override; // PC 0x005BD050
        void ShadowRenderCasters(unsigned int) override; // PC 0x00489F10
        void ShadowRenderReceivers() override; // PC 0x0048A010
        void ShadowRenderEnd() override; // PC 0x0048A130
        void SetScissorRect(float fLeft, float fRight, float fBottom, float fTop, bool bSetOrReset) override; // PC 0x00488E20
        bool MirrorRenderBegin() override; // PC 0x00489810
        void MirrorRenderReflectors() override; // PC 0x00489880
        void MirrorRenderReflected() override; // PC 0x004898C0
        bool MirrorRenderZHole() override; // PC 0x0048AD40
        void MirrorRenderEnd() override; // PC 0x00489900
        void* LockOffscreenSurface(unsigned int* pWidth, int* pHeight, int* pPitch, SZVRECT* pRect) override; // PC 0x004899B0
        void UnlockOffscreenSurface(void* pSurface) override; // PC 0x00489A10
        void BlitOffscreenSurface() override; // PC 0x00489A30
        bool CreateOffscreenSurface(int, int) override; // PC 0x00489930
        void DestroyOffscreenSurface() override; // PC 0x00489970
        void OnPaint(int) override; // PC 0x00488590 (implements the Wintel pure virtual)
        void OnSize(unsigned int lWidth, unsigned int lHeight) override; // PC 0x0048AA60
        void Clear() override; // PC 0x00489330
        void Init() override; // PC 0x0048B280 (implements the Wintel pure virtual)
        bool BackupBufferExists() override; // PC 0x004884F0 (returns m_field159C)
        int AdjustWindow(const char* pWindowTitle) override; // PC 0x00489310 (base + Clear)

        // ZRenderWintelD3D (new virtuals, PC vtable slots 156-159)
        virtual void OnCopyData(); // PC slot 156 (stub)
        virtual void MakeMe(HWND hWnd); // PC 0x00489270
        virtual unsigned int ShadowRenderCasterStart(unsigned int lCaster, unsigned int lPass, float fUnknown); // PC 0x00489C90
        virtual void PlotOffsetAlloc(int lX, int lY, const char* pszName, ZOffsetAlloc* pAllocator, uint32_t lTotalSize); // PC 0x00489670

        // methods
        ZRenderWintelD3D(int hInstance, void* hParentWnd); // PC 0x00488300
        static bool m_bInScene; // scene-in-progress flag (PC ZRenderWintelD3D::m_bInScene)
        void FreeViewBuffers(); // PC 0x004887E0
        void InitSwapChain();   // PC 0x004885D0
        void sub_48AB70();      // PC 0x0048AB70 (post-device-reset reallocation)
        static void HandleOverrideSLIOption(); // PC 0x00488420 (NVCPL SLI override)

        // members
        char m_field1524;                // +0x1524
        char m_field1525;                // +0x1525
        char field_1526;                 // +0x1526 (vendor workaround flag)
        char m_pOffscreenSurface;        // +0x1527 (offscreen surface state flag)
        int m_iOffscreenSurfaceWidth;     // +0x1528 (set by CreateOffscreenSurface)
        int m_iOffscreenSurfaceHeight;    // +0x152C
        RE_ADD_PADDING(0x24);             // +0x1530
        int m_field1554;                  // +0x1554 (caps flag & 0x20000)
        RE_ADD_PADDING(0x18);             // +0x1558
        int m_field1570;                  // +0x1570 (caps flag & 0x20000)
        RE_ADD_PADDING(0x28);             // +0x1574
        int m_field159C;                  // +0x159C (backup-buffer state, returned by BackupBufferExists)
        RE_ADD_PADDING(0x18);             // +0x15A0
        int m_field15B8;                  // +0x15B8 (shadow caster-pass flag & 0x100)
        RE_ADD_PADDING(0xA4);             // +0x15BC
        D3DPRESENT_PARAMETERS m_d3dPresentParameters; // +0x1660 (0x38 bytes)
        int uActiveAdapter;               // +0x1698
        int m_field169C;                  // +0x169C (adapter monitor handle)
        IDirect3DSwapChain9* m_pSwapChain; // +0x16A0
        IDirect3DSurface9* m_pSurface16A4; // +0x16A4
        IDirect3DSurface9* m_pSurface16A8; // +0x16A8
        char field_16AC;                  // +0x16AC
        char field_16AD;                  // +0x16AD
        char field_16AE;                  // +0x16AE
        char field_16AF;                  // +0x16AF
        int m_bDisableMMX;                // +0x16B0
        int m_field16B4;                  // +0x16B4 (zeroed by the ctor)
        IDirect3DTexture9* m_pTexture16B8; // +0x16B8
        IDirect3DSurface9* m_pSurface16BC; // +0x16BC
        IDirect3DSurface9* m_pAntialiasSurface; // +0x16C0
        RE_ADD_PADDING(0x28);             // +0x16C4
        IDirect3DTexture9* m_field16EC;   // +0x16EC (double-buffered offscreen texture 0)
        IDirect3DTexture9* m_field16F0;   // +0x16F0 (double-buffered offscreen texture 1)
        IDirect3DSurface9* m_field16F4;   // +0x16F4 (surface level of m_field16EC)
        IDirect3DSurface9* m_field16F8;   // +0x16F8 (surface level of m_field16F0)
        IDirect3DSurface9* m_field16FC;   // +0x16FC (offscreen depth/stencil surface)
        int m_field1700;                  // +0x1700 (zeroed by the ctor)
        IDirect3DTexture9* m_pBinkVideoTextureD3D; // +0x1704 (offscreen render-target texture)
        int m_field1708;                  // +0x1708 (vendor workaround flag)
        RE_ADD_PADDING(4);                // +0x170C
        int m_field1710;                  // +0x1710 (zeroed by the ctor)
        RE_ADD_PADDING(0x80);             // +0x1714
    };
    RE_VERIFY_SIZE(ZRenderWintelD3D, 0x1794); // Verified PC alloc
    RE_VERIFY_OFFSET(ZRenderWintelD3D, m_d3dPresentParameters, 0x1660); // Verified by ZRenderWintelD3D::Ctor
    RE_VERIFY_OFFSET(ZRenderWintelD3D, m_pSwapChain, 0x16A0);           // Verified by ZRenderWintelD3D::Ctor
    RE_VERIFY_OFFSET(ZRenderWintelD3D, m_pAntialiasSurface, 0x16C0);    // Verified by ZRenderWintelD3D::Ctor
    RE_VERIFY_OFFSET(ZRenderWintelD3D, m_pBinkVideoTextureD3D, 0x1704); // Verified by ZRenderWintelD3D::Ctor
}
