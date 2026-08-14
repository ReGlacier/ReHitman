#include <Glacier/Render/ZBlurTextureWintelD3D.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZDirect3DDevice.h>


namespace Glacier
{
    namespace
    {
        // PC .rdata:0x765D28 - matches the locked vertex format: float3 position + float2 texcoord
        D3DVERTEXELEMENT9 g_BlurVertexElements[] =
        {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };
    }

    ZBlurTextureWintelD3D::ZBlurTextureWintelD3D()
    {
        m_pRender = g_pSysInterface->WindowFirst;
        
        for (int i = 0; i < MAX_BLUR_PASSES_NR; ++i)
        {
            m_pTextures[i] = nullptr;
        }
        
        m_pQuadVB = nullptr;
        m_pVDKBLUR = nullptr;
        m_pPSBlur = nullptr;
        m_pPixelShader2 = nullptr;
        m_pPixelShader3 = nullptr;
        m_pVSBlur = nullptr;
        m_pVertexShader2 = nullptr;
    }

    ZBlurTextureWintelD3D::~ZBlurTextureWintelD3D()
    {
        FreeDeviceBuffers();
        m_pRender = nullptr;
    }

    void ZBlurTextureWintelD3D::Init(int lWidth, int lHeight)
    {
        m_lWidth = lWidth;
        m_lHeight = lHeight;
    }

    void ZBlurTextureWintelD3D::AllocateDeviceBuffers()
    {
        int lWidth = m_lWidth;
        if (lWidth <= 1)
            lWidth = 1;
        int lHeight = m_lHeight;
        if (lHeight <= 1)
            lHeight = 1;

        // Mip-like chain of render targets, each one half the previous size (up to 8 levels)
        m_iBlurMaxPasses = 0;
        while (true)
        {
            int lHalfWidth = lWidth >> 1;
            int lHalfHeight = lHeight >> 1;
            if (lHalfWidth == 1 || lHalfHeight == 1)
                break;
            g_pd3dDevice->CreateTexture(lHalfWidth, lHalfHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTextures[m_iBlurMaxPasses], nullptr);
            ++m_iBlurMaxPasses;
            if (m_iBlurMaxPasses >= MAX_BLUR_PASSES_NR)
                break;
            lWidth = lHalfWidth;
            lHeight = lHalfHeight;
        }

        // Fullscreen triangle strip: 4 vertices * (float3 position + float2 texcoord) = 0x50 bytes
        g_pd3dDevice->CreateVertexBuffer(0x50, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pQuadVB, nullptr);

        float* pVertices = nullptr;
        m_pQuadVB->Lock(0, 0x50, reinterpret_cast<void**>(&pVertices), 0);
        {
            // positions: (0,0,0), (2,0,0), (0,2,0), (2,2,0)
            pVertices[0] = 0.0f;  pVertices[1] = 0.0f;  pVertices[2] = 0.0f;
            pVertices[5] = 2.0f;  pVertices[6] = 0.0f;  pVertices[7] = 0.0f;
            pVertices[10] = 0.0f; pVertices[11] = 2.0f; pVertices[12] = 0.0f;
            pVertices[15] = 2.0f; pVertices[16] = 2.0f; pVertices[17] = 0.0f;
            // shift into clip space: (-1,-1) .. (1,1)
            for (int i = 0; i < 4; ++i)
            {
                pVertices[i * 5 + 0] -= 1.0f;
                pVertices[i * 5 + 1] -= 1.0f;
            }
            // texcoords: (0,1), (1,1), (0,0), (1,0)
            pVertices[3] = 0.0f;  pVertices[4] = 1.0f;
            pVertices[8] = 1.0f;  pVertices[9] = 1.0f;
            pVertices[13] = 0.0f; pVertices[14] = 0.0f;
            pVertices[18] = 1.0f; pVertices[19] = 0.0f;
        }
        m_pQuadVB->Unlock();

        // ps.1.1, 4-tap blur: r0.rgb = (t0 + t1 + t2 + t3) * c0, r0.a = c1.a
        DWORD dwPixelShader1[] =
        {
            0xFFFF0101, 0x00000042, 0xB00F0000, 0x00000042,
            0xB00F0001, 0x00000042, 0xB00F0002, 0x00000042,
            0xB00F0003, 0x00000005, 0x800F0001, 0xB0E40000,
            0xA0E40000, 0x00000004, 0x800F0000, 0xB0E40001,
            0xA0E40000, 0x80E40001, 0x00000004, 0x800F0001,
            0xB0E40002, 0xA0E40000, 0x80E40000, 0x00000004,
            0x800F0000, 0xB0E40003, 0xA0E40000, 0x80E40001,
            0x00000001, 0x80080000, 0xA0FF0001, 0x0000FFFF,
        };
        g_pd3dDevice->CreatePixelShader(dwPixelShader1, &m_pPSBlur);

        // ps.1.1, copy: r0 = t0
        DWORD dwPixelShader2[] =
        {
            0xFFFF0101, 0x00000042, 0xB00F0000, 0x00000001,
            0x800F0000, 0xB0E40000, 0x0000FFFF,
        };
        g_pd3dDevice->CreatePixelShader(dwPixelShader2, &m_pPixelShader2);

        // ps.1.1, bright-pass threshold: r0 = (t0.x >= 1.0) ? 1 : 0
        DWORD dwPixelShader3[] =
        {
            0xFFFF0101, 0x00000051, 0xA00F0000, 0x3F800000,
            0x00000000, 0x00000000, 0xBF800000, 0x00000051,
            0xA00F0001, 0x00000000, 0x00000000, 0x00000000,
            0x3F000000, 0x00000051, 0xA00F0002, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000051,
            0xA00F0003, 0x3F800000, 0x3F800000, 0x3F800000,
            0x3F800000, 0x00000042, 0xB00F0000, 0x00000008,
            0x800F0001, 0xA0E40000, 0xB0E40000, 0x00000002,
            0x80080001, 0x80FF0001, 0xA0FF0000, 0x00000004,
            0x80080000, 0x80FF0001, 0xA0FF0000, 0xA0FF0001,
            0x00000050, 0x800F0000, 0x80FF0000, 0xA0E40002,
            0xA0E40003, 0x0000FFFF,
        };
        g_pd3dDevice->CreatePixelShader(dwPixelShader3, &m_pPixelShader3);

        // vs.1.1, pass-through with a single generated texcoord
        DWORD dwVertexShader2[] =
        {
            0xFFFE0101, 0x00000051, 0xA00F0001, 0x3F800000,
            0x00000000, 0x3F000000, 0x00000000, 0x0000001F,
            0x80000000, 0x900F0000, 0x00000001, 0x80070000,
            0xA0E40001, 0x00000004, 0x80030000, 0xA0000000,
            0x80E40000, 0x80E90000, 0x00000004, 0x80030000,
            0x90E40000, 0xA0AA0001, 0x80E40000, 0x00000002,
            0x80040000, 0x81550000, 0xA0550000, 0x00000002,
            0xE0030000, 0x80E80000, 0xA0E20001, 0x00000004,
            0xC00F0000, 0x90240000, 0xA0400001, 0xA0150001,
            0x00000001, 0xD00F0000, 0xA0000001, 0x00000004,
            0xE00C0000, 0x90240000, 0xA0440001, 0xA0140001,
            0x0000FFFF,
        };
        g_pd3dDevice->CreateVertexShader(dwVertexShader2, &m_pVertexShader2);

        // vs.1.1, generates the 4 tap texcoords (oT0-oT3) for the blur pixel shader
        DWORD dwVertexShader1[] =
        {
            0xFFFE0101, 0x00000051, 0xA00F0003, 0x3F800000,
            0x00000000, 0x3F000000, 0x00000000, 0x0000001F,
            0x80000000, 0x900F0000, 0x0000001F, 0x80000005,
            0x900F0001, 0x00000001, 0x80070001, 0xA0E40003,
            0x00000004, 0x80030000, 0xA0000000, 0x80E40001,
            0x80E90001, 0x00000004, 0x80030000, 0x90E40000,
            0xA0AA0003, 0x80E40000, 0x00000002, 0x80040000,
            0x81550000, 0xA0550000, 0x00000002, 0x80030000,
            0x80E80000, 0xA0E20003, 0x00000004, 0x800C0000,
            0x90AA0000, 0xA0440003, 0xA0140003, 0x00000004,
            0xE00F0000, 0xA0040001, 0x80500001, 0x80E40000,
            0x00000004, 0xE00F0001, 0xA00E0001, 0x80500001,
            0x80E40000, 0x00000004, 0xE00F0002, 0xA0040002,
            0x80500001, 0x80E40000, 0x00000004, 0xE00F0003,
            0xA00E0002, 0x80500001, 0x80E40000, 0x00000004,
            0xC00F0000, 0x90A40000, 0xA0400003, 0xA0150003,
            0x00000001, 0xD00F0000, 0x90550001, 0x0000FFFF,
        };

        g_pd3dDevice->CreateVertexShader(dwVertexShader1, &m_pVSBlur);
        g_pd3dDevice->CreateVertexDeclaration(g_BlurVertexElements, &m_pVDKBLUR);
    }

    void ZBlurTextureWintelD3D::FreeDeviceBuffers()
    {
        if (!m_pRender)
            return;

        // Textures/VB/declaration are detached only when their refcount hits zero,
        // shaders are released and cleared unconditionally (as in the PC binary).
        for (int i = 0; i < 8; ++i)
        {
            if (m_pTextures[i] && m_pTextures[i]->Release() == 0)
                m_pTextures[i] = nullptr;
        }

        if (m_pQuadVB && m_pQuadVB->Release() == 0)
            m_pQuadVB = nullptr;

        if (m_pVDKBLUR && m_pVDKBLUR->Release() == 0)
            m_pVDKBLUR = nullptr;

        if (m_pPSBlur)
        {
            m_pPSBlur->Release();
            m_pPSBlur = nullptr;
        }

        if (m_pPixelShader2)
        {
            m_pPixelShader2->Release();
            m_pPixelShader2 = nullptr;
        }

        if (m_pPixelShader3)
        {
            m_pPixelShader3->Release();
            m_pPixelShader3 = nullptr;
        }

        if (m_pVSBlur)
        {
            m_pVSBlur->Release();
            m_pVSBlur = nullptr;
        }

        if (m_pVertexShader2)
        {
            m_pVertexShader2->Release();
            m_pVertexShader2 = nullptr;
        }
    }
}
