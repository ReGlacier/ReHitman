#include <Glacier/Render/Debug/ZDrawDebugRenderD3D.h>

#include <Glacier/Camera/ZCameraSpace.h>
#include <Glacier/Render/View/IView.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderWintelD3D.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniAssert.h>

#include <cmath>
#include <cstring>


namespace Glacier
{
    namespace
    {
        static constexpr const char* g_strPixelShaderProgram__3PBDB = R"(
sampler s : register(s0);

struct PS_IN
{
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PS_IN In) : COLOR0
{
    return In.Color * tex2D(s, In.TexCoord);
}
        )";

        // The vertex transform is stored as four independent rows (columns of the
        // row-vector world*projection matrix) so the column-major default of HLSL
        // matrices cannot reorder the registers.
        static constexpr const char* g_strVertexShaderProgram__3PBDB = R"(
float4 WVP0 : register(c0);
float4 WVP1 : register(c1);
float4 WVP2 : register(c2);
float4 WVP3 : register(c3);

struct VS_IN
{
    float4 ObjPos   : POSITION;
    float4 Color    : COLOR0;
    float3 TexCoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 ProjPos  : POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

VS_OUT main(VS_IN In)
{
    VS_OUT Out;

    float4 pos = float4(In.ObjPos.xyz, 1.0);

    Out.ProjPos.x = dot(pos, WVP0);
    Out.ProjPos.y = dot(pos, WVP1);
    Out.ProjPos.z = dot(pos, WVP2);
    Out.ProjPos.w = dot(pos, WVP3);
    Out.Color     = In.Color;
    Out.TexCoord  = In.TexCoord.xy;

    return Out;
}
)";

        // SVertex: float3 position, uint32 colour, float3 texcoord (stride 28).
        D3DVERTEXELEMENT9 g_DebugVertexElements[] =
        {
            { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
            { 0, 16, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        void Multiply4x4(float* pOut, const float* pA, const float* pB)
        {
            for (int r = 0; r < 4; ++r)
            {
                for (int c = 0; c < 4; ++c)
                {
                    pOut[r * 4 + c] =
                        pA[r * 4 + 0] * pB[0 * 4 + c] +
                        pA[r * 4 + 1] * pB[1 * 4 + c] +
                        pA[r * 4 + 2] * pB[2 * 4 + c] +
                        pA[r * 4 + 3] * pB[3 * 4 + c];
                }
            }
        }
    }

    ZDrawDebugRenderD3D::ZDrawDebugRenderD3D(ZRenderWintelD3D* pRender)
    {
        m_pRender = pRender;
        m_pFontTexture = nullptr;
        m_pVertexShader = nullptr;
        m_pPixelShader = nullptr;
        m_pVertexDecl = nullptr;
    }

    ZDrawDebugRenderD3D::~ZDrawDebugRenderD3D()
    {
        Free();
    }

    void ZDrawDebugRenderD3D::Allocate()
    {
        ID3DXBuffer* pShaderBuffer = nullptr;
        ID3DXBuffer* pErrorBuffer = nullptr;

        if (SUCCEEDED(D3DXCompileShader(g_strVertexShaderProgram__3PBDB,
                static_cast<UINT>(strlen(g_strVertexShaderProgram__3PBDB)),
                nullptr, nullptr, "main", "vs_2_0", 0, &pShaderBuffer, &pErrorBuffer, nullptr)))
        {
            g_pd3dDevice->CreateVertexShader(static_cast<DWORD*>(pShaderBuffer->GetBufferPointer()), &m_pVertexShader);
            pShaderBuffer->Release();
            pShaderBuffer = nullptr;
        }

        if (SUCCEEDED(D3DXCompileShader(g_strPixelShaderProgram__3PBDB,
                static_cast<UINT>(strlen(g_strPixelShaderProgram__3PBDB)),
                nullptr, nullptr, "main", "ps_2_0", 0, &pShaderBuffer, &pErrorBuffer, nullptr)))
        {
            g_pd3dDevice->CreatePixelShader(static_cast<DWORD*>(pShaderBuffer->GetBufferPointer()), &m_pPixelShader);
            pShaderBuffer->Release();
            pShaderBuffer = nullptr;
        }

        if (pErrorBuffer)
            pErrorBuffer->Release();

        g_pd3dDevice->CreateVertexDeclaration(g_DebugVertexElements, &m_pVertexDecl);
    }

    void ZDrawDebugRenderD3D::Free()
    {
        g_pd3dDevice->SetVertexShader(nullptr);
        g_pd3dDevice->SetPixelShader(nullptr);
        g_pd3dDevice->SetVertexDeclaration(nullptr);

        if (m_pVertexShader)
        {
            m_pVertexShader->Release();
            m_pVertexShader = nullptr;
        }

        if (m_pPixelShader)
        {
            m_pPixelShader->Release();
            m_pPixelShader = nullptr;
        }

        if (m_pVertexDecl)
        {
            m_pVertexDecl->Release();
            m_pVertexDecl = nullptr;
        }
    }

    void ZDrawDebugRenderD3D::Invalidate()
    {
        Free();
    }

    void ZDrawDebugRenderD3D::Begin(IView* pIView, ZCameraSpace* pCameraSpace)
    {
        ZDrawDebugRender::Begin(pIView, pCameraSpace);

        SetDrawMode(0);
        SetTexture(0u);

        const uint32_t lSizeX = static_cast<uint32_t>(m_pRender->GetSizeX());
        const uint32_t lSizeY = static_cast<uint32_t>(m_pRender->GetSizeY());

        ZMat4x4 matProj;

        if (pCameraSpace)
        {
            const uint32_t aViewport[4] =
            {
                m_Viewport.x,
                m_Viewport.y,
                m_Viewport.x + m_Viewport.w,
                m_Viewport.y + m_Viewport.h,
            };

            m_pRender->CreateFrustumFromCameraSpace(&matProj, pCameraSpace, aViewport, true, false);
        }
        else
        {
            SViewport viewport;
            viewport.x = 0;
            viewport.y = 0;
            viewport.w = lSizeX;
            viewport.h = lSizeY;
            SetViewport(viewport);

            // Orthographic projection over [0..size] with a [-5,5] depth range.
            for (float& f : matProj.data)
                f = 0.0f;

            matProj.data[0] = 2.0f / static_cast<float>(lSizeX);
            matProj.data[5] = 2.0f / static_cast<float>(lSizeY);
            matProj.data[10] = 0.1f;
            matProj.data[12] = -1.0f;
            matProj.data[13] = -1.0f;
            matProj.data[14] = 0.5f;
            matProj.data[15] = 1.0f;
        }

        D3DVIEWPORT9 d3dViewport;
        d3dViewport.X = m_Viewport.x;
        d3dViewport.Y = m_Viewport.y;
        d3dViewport.Width = m_Viewport.w;
        d3dViewport.Height = m_Viewport.h;
        d3dViewport.MinZ = 0.0f;
        d3dViewport.MaxZ = 1.0f;
        g_pd3dDevice->SetViewport(&d3dViewport);

        // World-to-view in Glacier row-vector layout.
        float aWorldToView[16];
        aWorldToView[0] = m_WorldToView.m0.data[0];
        aWorldToView[1] = m_WorldToView.m0.data[1];
        aWorldToView[2] = m_WorldToView.m0.data[2];
        aWorldToView[3] = 0.0f;
        aWorldToView[4] = m_WorldToView.m0.data[3];
        aWorldToView[5] = m_WorldToView.m0.data[4];
        aWorldToView[6] = m_WorldToView.m0.data[5];
        aWorldToView[7] = 0.0f;
        aWorldToView[8] = m_WorldToView.m0.data[6];
        aWorldToView[9] = m_WorldToView.m0.data[7];
        aWorldToView[10] = m_WorldToView.m0.data[8];
        aWorldToView[11] = 0.0f;
        aWorldToView[12] = m_WorldToView.p0.x;
        aWorldToView[13] = m_WorldToView.p0.y;
        aWorldToView[14] = m_WorldToView.p0.z;
        aWorldToView[15] = 1.0f;

        float aWorldViewProj[16];
        Multiply4x4(aWorldViewProj, aWorldToView, matProj.data);

        // The shader computes dot(pos, register) so each register must hold a column of
        // the row-vector world*projection matrix.
        float aConstants[4][4];
        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 4; ++c)
                aConstants[r][c] = aWorldViewProj[r * 4 + c];
        }

        float aColumns[16];
        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
                aColumns[c * 4 + r] = aConstants[r][c];
        }

        g_pd3dDevice->SetVertexShaderConstantF(0, aColumns, 4);

        g_pd3dDevice->SetVertexShader(m_pVertexShader);
        g_pd3dDevice->SetPixelShader(m_pPixelShader);
        g_pd3dDevice->SetVertexDeclaration(m_pVertexDecl);

        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }

    void ZDrawDebugRenderD3D::End()
    {
        ZDrawDebugRender::End();
    }

    void ZDrawDebugRenderD3D::DrawPrimitives(D3DPRIMITIVETYPE eType, const SVertex* pVertices, uint32_t lNumPrimitives)
    {
        g_pd3dDevice->DrawPrimitiveUP(eType, lNumPrimitives, pVertices, sizeof(SVertex));
    }

    void ZDrawDebugRenderD3D::DrawLines(const SVertex* pVertices, uint32_t lNumLines)
    {
        DrawPrimitives(D3DPT_LINELIST, pVertices, lNumLines);
    }

    void ZDrawDebugRenderD3D::DrawTriangles(const SVertex* pVertices, uint32_t lNumTriangles)
    {
        DrawPrimitives(D3DPT_TRIANGLELIST, pVertices, lNumTriangles);
    }

    void ZDrawDebugRenderD3D::SetDrawMode(uint32_t lDrawMode)
    {
        if ((lDrawMode & 0x402603) != 0)
        {
            const bool bCounterClockWise = (lDrawMode & 0x400) != 0 || (lDrawMode & 0x2) == 0;
            g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, bCounterClockWise ? D3DCULL_CCW : D3DCULL_CW);
        }
        else
        {
            g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        }

        g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, (lDrawMode & 0x20000) ? TRUE : FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, (lDrawMode & 0x80000) ? D3DZB_FALSE : D3DZB_TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    void ZDrawDebugRenderD3D::SetTexture(ZTextureBase* pTexture)
    {
        IDirect3DBaseTexture9* pD3DTexture = nullptr;
        if (pTexture)
            pD3DTexture = static_cast<IDirect3DBaseTexture9*>(pTexture->m_pUserData);

        g_pd3dDevice->SetTexture(0, pD3DTexture);
    }

    void ZDrawDebugRenderD3D::SetTexture(uint32_t lTextureId)
    {
        ZTextureBase* pTexture = nullptr;

        if (lTextureId > static_cast<uint32_t>(TEXTURE_MOUSE))
        {
            if (lTextureId == static_cast<uint32_t>(TEXTURED3D_SHADOWMAPCUBECOLOR))
                pTexture = &g_texShadowCubeMapColor;
            else
                pTexture = g_pRenderDll->m_pTexCon->GetTexture(lTextureId, 0);
        }
        else if (lTextureId == static_cast<uint32_t>(TEXTURE_MOUSE))
        {
            pTexture = &g_texMouse16x16;
        }
        else if (lTextureId == static_cast<uint32_t>(TEXTURE_NONE))
        {
            pTexture = &g_texWhite;
        }
        else if (lTextureId == static_cast<uint32_t>(TEXTURE_FONT))
        {
            pTexture = &g_texFont8x13;
        }
        else
        {
            pTexture = g_pRenderDll->m_pTexCon->GetTexture(lTextureId, 0);
        }

        SetTexture(pTexture);
    }
}
