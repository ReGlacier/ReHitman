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

        // TBD
        // ...
        HRESULT Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
        HRESULT Clear(DWORD dwCount, const D3DRECT* Rect, DWORD dwFlags, D3DCOLOR Color, float fZ, DWORD dwStencil);
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
        void ResetState();

        // members
        IDirect3DDevice9* m_pDevice { nullptr }; // +0x4
        LONG m_lRefCount { 0 }; // +0x8
    };
    RE_VERIFY_SIZE(ZDirect3DDevice, 0xC); // Verified PC allocation
    RE_VERIFY_OFFSET(ZDirect3DDevice, m_pDevice, 0x4); // Verified
    RE_VERIFY_OFFSET(ZDirect3DDevice, m_lRefCount, 0x8); // Verified (AddRef/SubRef work with it)
}
