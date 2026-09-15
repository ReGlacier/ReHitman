#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Debug/ZDrawDebugRender.h>
#include <Glacier/Render/Fwd.h>


namespace Glacier
{
    class ZDrawDebugRenderD3D : public ZDrawDebugRender
    {
    public:
        // types
        enum TEXTURETYPED3D 
        {
            TEXTURED3D_SHADOWCLIP = 0x80000002,
            TEXTURED3D_WHITE = 0x80000003,
            TEXTURED3D_NORMALIZER = 0x80000004,
            TEXTURED3D_SPECULARMAP = 0x80000005,
            TEXTURED3D_SCATTERMAP = 0x80000006,
            TEXTURED3D_HGPHASEMAP = 0x80000007,
            TEXTURED3D_REFRACTIONMAP = 0x80000008,
            TEXTURED3D_SHADOWMAPCUBECOLOR = 0x80000009,
        };

        struct SD3DVertex
        {
            ZVector3 p;
            uint32_t c;
            ZVector3 t;
        };

        // vtbl
        ~ZDrawDebugRenderD3D() override;
        void xFlush() override;
        void Begin(IView* pView, ZCameraSpace* pCameraSpace) override;
        void End() override;
        void DrawLines() override;
        void DrawTriangles() override;
        void SetDrawMode(uint32_t lMode) override;
        void SetTexture(ZTextureBase* pTexture) override;
        void SetTexture(uint32_t lTextureId) override;

        // methods
        void Allocate();
        void Invalidate();
        void Free();

        // members
        IDirect3DTexture9* m_pFontTexture;
        IDirect3DVertexShader9* m_pVertexShader;
        IDirect3DPixelShader9* m_pPixelShader;
        IDirect3DVertexDeclaration9* m_pVertexDecl;
        ZRenderWintelD3D* m_pRender;
    };
}