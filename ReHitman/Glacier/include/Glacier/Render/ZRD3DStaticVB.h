#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRD3DStaticVB
    {
    public:
        // methods
        ZRD3DStaticVB(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool);
        ~ZRD3DStaticVB();

        IDirect3DVertexBuffer9* Interface();
        void* LockedData();

        // members
        IDirect3DVertexBuffer9* m_pVertexBuffer;
        void* m_pLockedData; // also serves as "buffer is locked" flag
        uint32_t m_lLength;
    };
    RE_VERIFY_SIZE(ZRD3DStaticVB, 0xC); // Verified
}