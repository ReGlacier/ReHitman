#pragma once

#include <d3d9.h>

namespace Glacier {
    class ZRenderWintelD3D;

    class ZDrawSurfaceD3D {
    public:
        //vftable
        virtual void Release(bool bFreeMem);
        virtual void Begin();
        virtual void End();
        virtual void Setup(int iOffsetX, int iOffsetY, int iWidth, int iHeight);

        //data (total size is 0x38)
        int m_isLocked;
        int m_field8; //???
        IDirect3DSurface9* m_pNewRenderTarget; //+0xC
        IDirect3DSurface9* m_pNewDepthStencilBuffer; //+0x10
        IDirect3DSurface9* m_pOldRenderTarget; //+0x14
        IDirect3DSurface9* m_pOldDepthStencilBuffer; //+0x18
        D3DVIEWPORT9 m_sViewport; //+0x1C
        ZRenderWintelD3D m_pRender; //+0x34
    };
}