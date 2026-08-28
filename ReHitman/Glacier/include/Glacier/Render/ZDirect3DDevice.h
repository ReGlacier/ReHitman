#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <cstdint>


namespace Glacier
{
    // PC RTTI: ZDirect3DDevice : ID3DXEffectStateManager, IUnknown
    // vtbl 0x0076419C - exactly the 21 methods of ID3DXEffectStateManager (no dtor slot)
    class ZDirect3DDevice : public ID3DXEffectStateManager
    {
    public:
        // vtbl
        STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override;
        STDMETHOD_(ULONG, AddRef)() override;
        STDMETHOD_(ULONG, Release)() override;
        STDMETHOD(SetTransform)(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix) override;
        STDMETHOD(SetMaterial)(const D3DMATERIAL9* pMaterial) override;
        STDMETHOD(SetLight)(DWORD Index, const D3DLIGHT9* pLight) override;
        STDMETHOD(LightEnable)(DWORD Index, BOOL Enable) override;
        STDMETHOD(SetRenderState)(D3DRENDERSTATETYPE State, DWORD Value) override;
        STDMETHOD(SetTexture)(DWORD Stage, IDirect3DBaseTexture9* pTexture) override;
        STDMETHOD(SetTextureStageState)(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override;
        STDMETHOD(SetSamplerState)(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override;
        STDMETHOD(SetNPatchMode)(FLOAT NumSegments) override;
        STDMETHOD(SetFVF)(DWORD FVF) override;
        STDMETHOD(SetVertexShader)(IDirect3DVertexShader9* pShader) override;
        STDMETHOD(SetVertexShaderConstantF)(UINT Register, const FLOAT* pConstantData, UINT RegisterCount) override;
        STDMETHOD(SetVertexShaderConstantI)(UINT Register, const INT* pConstantData, UINT RegisterCount) override;
        STDMETHOD(SetVertexShaderConstantB)(UINT Register, const BOOL* pConstantData, UINT RegisterCount) override;
        STDMETHOD(SetPixelShader)(IDirect3DPixelShader9* pShader) override;
        STDMETHOD(SetPixelShaderConstantF)(UINT Register, const FLOAT* pConstantData, UINT RegisterCount) override;
        STDMETHOD(SetPixelShaderConstantI)(UINT Register, const INT* pConstantData, UINT RegisterCount) override;
        STDMETHOD(SetPixelShaderConstantB)(UINT Register, const BOOL* pConstantData, UINT RegisterCount) override;

        // methods
        ZDirect3DDevice(IDirect3DDevice9* pDevice);

        HRESULT BeginScene();          // PC 0x004903F0
        HRESULT EndScene();            // PC 0x0048FA60
        HRESULT CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentParameters, IDirect3DSwapChain9** ppSwapChain); // PC 0x00490430
        HRESULT GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** ppSwapChain); // PC 0x004903A0
        HRESULT CreateRenderTarget(UINT iWidth, UINT iHeight, D3DFORMAT format, D3DMULTISAMPLE_TYPE multiSample, DWORD msQuality, BOOL bLockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle); // PC 0x004902F0
        HRESULT ColorFill(IDirect3DSurface9* pSurface, const RECT* pRect, D3DCOLOR dwColor); // PC 0x0048F990
        HRESULT TestCooperativeLevel(); // PC 0x0048F760

        // TBD
        // ...
        HRESULT Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
        HRESULT Clear(DWORD dwCount, const D3DRECT* Rect, DWORD dwFlags, D3DCOLOR Color, float fZ, DWORD dwStencil);
        HRESULT StretchRect(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestSurface, const RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
        HRESULT SetRenderTarget(DWORD dwRenderTargetIndex, IDirect3DSurface9* pRenderTarget);
        HRESULT GetRenderTarget(DWORD dwRenderTargetIndex, IDirect3DSurface9** ppRenderTarget);
        HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
        HRESULT GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface);
        HRESULT SetViewport(const D3DVIEWPORT9* pViewport);
        HRESULT GetViewport(D3DVIEWPORT9* pViewport);
        HRESULT SetScissorRect(const RECT* pRect);
        HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
        HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride); // PC 0x00490380
        HRESULT CreateOffscreenPlainSurface(UINT iWidth, UINT iHeight, D3DFORMAT format, D3DPOOL pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle); // PC 0x004904A0

        // PC 0x004A4D50 (static; draws a textured quad plane through g_pVBPDT1)
        static void DrawPlane(ZDirect3DDevice* pD3DDev, float x, float y, float w, float h, D3DCOLOR color, float z, float uMax, float vMax);
        HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);
        HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexBuffer);
        HRESULT SetStreamSource(uint32_t lStreamNumber, IDirect3DVertexBuffer9* pVertexBuffer, uint32_t lOffset, uint32_t lStride);
        HRESULT CreateTexture(UINT iWidth, UINT iHeight, UINT iLevels, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture9** pOutTexture, HANDLE* pHandle);
        HRESULT CreateCubeTexture(UINT iEdgeLength, UINT iLevels, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool, IDirect3DCubeTexture9** pOutTexture, HANDLE* pHandle);
        HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** pOutVertexBuffer, HANDLE* pHandle);
        HRESULT CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** pOutIndexBuffer, HANDLE* pHandle);
        HRESULT CreateVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl);
        HRESULT CreateVertexShader(DWORD* pFunction, IDirect3DVertexShader9** ppShader);
        HRESULT CreatePixelShader(DWORD* pFunction, IDirect3DPixelShader9** ppShader);
        HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE ePrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
        HRESULT CreateDepthStencilSurface(DWORD lWidth, DWORD lHeight, _D3DFORMAT Format, _D3DMULTISAMPLE_TYPE MutlisampleType, DWORD Quality, bool Discard, IDirect3DSurface9** ppSurface, HANDLE* pHandle);
        HRESULT GetDeviceCaps(D3DCAPS9* pCaps);
        void SetGammaRamp(UINT iSwapChain, DWORD dwFlags, const D3DGAMMARAMP* pRamp); // PC 0x0048F490
        void ResetState();

        // members
        IDirect3DDevice9* m_pDevice { nullptr }; // +0x4
        LONG m_lRefCount { 0 }; // +0x8
    };
    RE_VERIFY_SIZE(ZDirect3DDevice, 0xC); // Verified PC allocation
    RE_VERIFY_OFFSET(ZDirect3DDevice, m_pDevice, 0x4); // Verified
    RE_VERIFY_OFFSET(ZDirect3DDevice, m_lRefCount, 0x8); // Verified (AddRef/SubRef work with it)
}
