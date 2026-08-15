#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    namespace
    {
        // State-filtering caches of the original ZDirect3DDevice (live in game .data).
        // Layout: SStateCacheEntry records are shared by the render-state and sampler-state tables.
        struct SStateCacheEntry
        {
            uint32_t dwValue;       // +0x0
            uint32_t dwCallCount;   // +0x4
            uint32_t dwSetCount;    // +0x8
            uint32_t dwUnused;      // +0xC
            uint32_t dwEnabled;     // +0x10 (asserted non-zero on use)
        };
        static_assert(sizeof(SStateCacheEntry) == 0x14);

        constexpr uint32_t kRenderStateCount = 0xD1; // PC SetRenderState bound check
        struct SRenderStateCache
        {
            SStateCacheEntry aEntries[kRenderStateCount];
        };
        static_assert(sizeof(SRenderStateCache) == 0x1054); // ends exactly at 0x90BF9C

        constexpr uint32_t kTextureStageCount = 0x10; // PC SetTexture/SetTextureStageState bound check
        constexpr uint32_t kTextureStageStateSlots = 33; // dwords per stage, indexed by D3DTEXTURESTAGESTATETYPE
        struct STextureStageCacheEntry
        {
            IDirect3DBaseTexture9* pTexture; // +0x0
            uint32_t dwStateValues[kTextureStageStateSlots]; // +0x4
        };
        static_assert(sizeof(STextureStageCacheEntry) == 0x88);

        struct STextureStateCache
        {
            uint32_t dwSetTextureCalls; // +0x0
            uint32_t dwSetTextureSets; // +0x4
            uint32_t dwSetTextureStageStateCalls; // +0x8
            uint32_t dwSetTextureStageStateSets; // +0xC
            STextureStageCacheEntry aStages[kTextureStageCount]; // +0x10
        };
        static_assert(sizeof(STextureStateCache) == 0x890); // ends exactly at 0x90C82C

        constexpr uint32_t kSamplerCount = 0x8; // PC SetSamplerState bound check
        constexpr uint32_t kSamplerStateRecords = 13; // 65 dwords per sampler
        struct SSamplerStateCache
        {
            SStateCacheEntry aEntries[kSamplerCount * kSamplerStateRecords];
        };
        static_assert(sizeof(SSamplerStateCache) == 0x820);

        STATIC_GLOBAL_VAR(SRenderStateCache, g_RenderStateCache, 0x90AF48, {});
        STATIC_GLOBAL_VAR(STextureStateCache, g_TextureStateCache, 0x90BF9C, {});
        STATIC_GLOBAL_VAR(SSamplerStateCache, g_SamplerStateCache, 0x90C82C, {});
    }

    ZDirect3DDevice::ZDirect3DDevice(IDirect3DDevice9* pDevice)
        : m_pDevice(pDevice)
        , m_lRefCount(0)
    {
    }

    STDMETHODIMP ZDirect3DDevice::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == IID_IUnknown || riid == IID_ID3DXEffectStateManager)
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }

        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) ZDirect3DDevice::AddRef()
    {
        return InterlockedIncrement(&m_lRefCount);
    }

    STDMETHODIMP_(ULONG) ZDirect3DDevice::Release()
    {
        const LONG lRefCount = InterlockedDecrement(&m_lRefCount);
        if (lRefCount)
        {
            return lRefCount;
        }

        delete this;
        return 0;
    }

    STDMETHODIMP ZDirect3DDevice::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix)
    {
        return m_pDevice->SetTransform(State, pMatrix);
    }

    STDMETHODIMP ZDirect3DDevice::SetMaterial(const D3DMATERIAL9* pMaterial)
    {
        return m_pDevice->SetMaterial(pMaterial);
    }

    STDMETHODIMP ZDirect3DDevice::SetLight(DWORD Index, const D3DLIGHT9* pLight)
    {
        return m_pDevice->SetLight(Index, pLight);
    }

    STDMETHODIMP ZDirect3DDevice::LightEnable(DWORD Index, BOOL Enable)
    {
        return m_pDevice->LightEnable(Index, Enable);
    }

    STDMETHODIMP ZDirect3DDevice::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
    {
        if (State >= kRenderStateCount)
        {
            ZASSERT(false);
        }

        SStateCacheEntry& entry = g_RenderStateCache.aEntries[State];
        if (!entry.dwEnabled)
        {
            ZASSERT(false);
        }

        ++entry.dwCallCount;
        if (entry.dwValue == Value)
        {
            return S_OK;
        }

        ++entry.dwSetCount;
        entry.dwValue = Value;
        return m_pDevice->SetRenderState(State, Value);
    }

    STDMETHODIMP ZDirect3DDevice::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
    {
        if (Stage >= kTextureStageCount)
        {
            ZASSERT(false);
        }

        STextureStageCacheEntry& stage = g_TextureStateCache.aStages[Stage];
        ++g_TextureStateCache.dwSetTextureCalls;
        if (stage.pTexture == pTexture)
        {
            return S_OK;
        }

        stage.pTexture = pTexture;
        ++g_TextureStateCache.dwSetTextureSets;
        return m_pDevice->SetTexture(Stage, pTexture);
    }

    STDMETHODIMP ZDirect3DDevice::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
    {
        if (Stage >= kTextureStageCount)
        {
            ZASSERT(false);
        }

        STextureStageCacheEntry& stage = g_TextureStateCache.aStages[Stage];
        ++g_TextureStateCache.dwSetTextureStageStateCalls;
        if (stage.dwStateValues[Type] == Value)
        {
            return S_OK;
        }

        stage.dwStateValues[Type] = Value;
        ++g_TextureStateCache.dwSetTextureStageStateSets;
        return m_pDevice->SetTextureStageState(Stage, Type, Value);
    }

    STDMETHODIMP ZDirect3DDevice::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
    {
        if (Sampler >= kSamplerCount)
        {
            ZASSERT(false);
        }

        SStateCacheEntry& entry = g_SamplerStateCache.aEntries[Sampler * kSamplerStateRecords + Type];
        if (!entry.dwEnabled)
        {
            ZASSERT(false);
        }

        ++entry.dwCallCount;
        if (entry.dwValue == Value)
        {
            return S_OK;
        }

        ++entry.dwSetCount;
        entry.dwValue = Value;
        return m_pDevice->SetSamplerState(Sampler, Type, Value);
    }

    STDMETHODIMP ZDirect3DDevice::SetNPatchMode(FLOAT NumSegments)
    {
        return m_pDevice->SetNPatchMode(NumSegments);
    }

    STDMETHODIMP ZDirect3DDevice::SetFVF(DWORD FVF)
    {
        return m_pDevice->SetFVF(FVF);
    }

    STDMETHODIMP ZDirect3DDevice::SetVertexShader(IDirect3DVertexShader9* pShader)
    {
        return m_pDevice->SetVertexShader(pShader);
    }

    STDMETHODIMP ZDirect3DDevice::SetVertexShaderConstantF(UINT Register, const FLOAT* pConstantData, UINT RegisterCount)
    {
        return m_pDevice->SetVertexShaderConstantF(Register, pConstantData, RegisterCount);
    }

    STDMETHODIMP ZDirect3DDevice::SetVertexShaderConstantI(UINT Register, const INT* pConstantData, UINT RegisterCount)
    {
        return m_pDevice->SetVertexShaderConstantI(Register, pConstantData, RegisterCount);
    }

    STDMETHODIMP ZDirect3DDevice::SetVertexShaderConstantB(UINT Register, const BOOL* pConstantData, UINT RegisterCount)
    {
        return m_pDevice->SetVertexShaderConstantB(Register, pConstantData, RegisterCount);
    }

    STDMETHODIMP ZDirect3DDevice::SetPixelShader(IDirect3DPixelShader9* pShader)
    {
        return m_pDevice->SetPixelShader(pShader);
    }

    STDMETHODIMP ZDirect3DDevice::SetPixelShaderConstantF(UINT Register, const FLOAT* pConstantData, UINT RegisterCount)
    {
        return m_pDevice->SetPixelShaderConstantF(Register, pConstantData, RegisterCount);
    }

    STDMETHODIMP ZDirect3DDevice::SetPixelShaderConstantI(UINT Register, const INT* pConstantData, UINT RegisterCount)
    {
        return m_pDevice->SetPixelShaderConstantI(Register, pConstantData, RegisterCount);
    }

    STDMETHODIMP ZDirect3DDevice::SetPixelShaderConstantB(UINT Register, const BOOL* pConstantData, UINT RegisterCount)
    {
        return m_pDevice->SetPixelShaderConstantB(Register, pConstantData, RegisterCount);
    }

    HRESULT ZDirect3DDevice::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
    {
        return m_pDevice->Reset(pPresentationParameters);
    }

    HRESULT ZDirect3DDevice::Clear(DWORD dwCount, const D3DRECT* pRect, DWORD dwFlags, D3DCOLOR Color, float fZ, DWORD dwStencil)
    {
        return m_pDevice->Clear(dwCount, pRect, dwFlags, Color, fZ, dwStencil);
    }

    HRESULT ZDirect3DDevice::StretchRect(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestSurface, const RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
    {
        return m_pDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    }

    HRESULT ZDirect3DDevice::SetRenderTarget(DWORD dwRenderTargetIndex, IDirect3DSurface9* pRenderTarget)
    {
        return m_pDevice->SetRenderTarget(dwRenderTargetIndex, pRenderTarget);
    }

    HRESULT ZDirect3DDevice::GetRenderTarget(DWORD dwRenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
    {
        return m_pDevice->GetRenderTarget(dwRenderTargetIndex, ppRenderTarget);
    }

    HRESULT ZDirect3DDevice::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
    {
        return m_pDevice->SetDepthStencilSurface(pNewZStencil);
    }

    HRESULT ZDirect3DDevice::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
    {
        return m_pDevice->GetDepthStencilSurface(ppZStencilSurface);
    }

    HRESULT ZDirect3DDevice::SetViewport(const D3DVIEWPORT9* pViewport)
    {
        return m_pDevice->SetViewport(pViewport);
    }

    HRESULT ZDirect3DDevice::GetViewport(D3DVIEWPORT9* pViewport)
    {
        return m_pDevice->GetViewport(pViewport);
    }

    HRESULT ZDirect3DDevice::SetScissorRect(const RECT* pRect)
    {
        return m_pDevice->SetScissorRect(pRect);
    }

    HRESULT ZDirect3DDevice::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
    {
        return m_pDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }

    HRESULT ZDirect3DDevice::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
    {
        return m_pDevice->SetVertexDeclaration(pDecl);
    }

    HRESULT ZDirect3DDevice::SetIndices(IDirect3DIndexBuffer9* pIndexBuffer)
    {
        return m_pDevice->SetIndices(pIndexBuffer);
    }

    HRESULT ZDirect3DDevice::SetStreamSource(uint32_t lStreamNumber, IDirect3DVertexBuffer9* pVertexBuffer, uint32_t lOffset, uint32_t lStride)
    {
        return m_pDevice->SetStreamSource(lStreamNumber, pVertexBuffer, lOffset, lStride);
    }

    HRESULT ZDirect3DDevice::CreateTexture(UINT iWidth, UINT iHeight, UINT iLevels, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture9** pOutTexture, HANDLE* pHandle)
    {
        return m_pDevice->CreateTexture(iWidth, iHeight, iLevels, dwUsage, format, pool, pOutTexture, pHandle);
    }

    HRESULT ZDirect3DDevice::CreateCubeTexture(UINT iEdgeLength, UINT iLevels, DWORD dwUsage, D3DFORMAT format, D3DPOOL pool, IDirect3DCubeTexture9** pOutTexture, HANDLE* pHandle)
    {
        return m_pDevice->CreateCubeTexture(iEdgeLength, iLevels, dwUsage, format, pool, pOutTexture, pHandle);
    }

    HRESULT ZDirect3DDevice::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** pOutVertexBuffer, HANDLE* pHandle)
    {
        return m_pDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, pOutVertexBuffer, pHandle);
    }

    HRESULT ZDirect3DDevice::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** pOutIndexBuffer, HANDLE* pHandle)
    {
        return m_pDevice->CreateIndexBuffer(Length, Usage, Format, Pool, pOutIndexBuffer, pHandle);
    }

    HRESULT ZDirect3DDevice::CreateVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
    {
        return m_pDevice->CreateVertexDeclaration(pVertexElements, ppDecl);
    }

    HRESULT ZDirect3DDevice::CreateVertexShader(DWORD* pFunction, IDirect3DVertexShader9** ppShader)
    {
        return m_pDevice->CreateVertexShader(pFunction, ppShader);
    }

    HRESULT ZDirect3DDevice::CreatePixelShader(DWORD* pFunction, IDirect3DPixelShader9** ppShader)
    {
        return m_pDevice->CreatePixelShader(pFunction, ppShader);
    }
    
    HRESULT ZDirect3DDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE ePrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
    {
        return m_pDevice->DrawIndexedPrimitive(ePrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
    }

    HRESULT ZDirect3DDevice::CreateDepthStencilSurface(DWORD lWidth, DWORD lHeight, _D3DFORMAT Format, _D3DMULTISAMPLE_TYPE MutlisampleType, DWORD Quality, bool Discard, IDirect3DSurface9** ppSurface, HANDLE* pHandle)
    {
        return m_pDevice->CreateDepthStencilSurface(lWidth, lHeight, Format, MutlisampleType, Quality, Discard, ppSurface, pHandle);
    }

    HRESULT ZDirect3DDevice::GetDeviceCaps(D3DCAPS9* pCaps)
    {
        return m_pDevice->GetDeviceCaps(pCaps);
    }

    void ZDirect3DDevice::ResetState()
    {
        static_assert((D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1) == 0x144);

        g_pd3dDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ZENABLE, 0);
        g_pd3dDevice->SetVertexShader(nullptr);
        g_pd3dDevice->SetPixelShader(nullptr);
        g_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        SetIndices(nullptr);
        g_pd3dDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHABLENDENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pd3dDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, 0);
        g_pd3dDevice->SetTexture(0, nullptr);
        g_pd3dDevice->SetTexture(1, nullptr);
        g_pd3dDevice->SetTexture(2, nullptr);
        g_pd3dDevice->SetTexture(3, nullptr);
    }
}
